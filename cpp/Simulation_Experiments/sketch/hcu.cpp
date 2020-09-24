#include "hcu.h"
#include "utils.h"
#include <cstdio>
#include <cmath>
#include <cstring>
#include <climits>

HCUSketch::HCUSketch(int size, int num_hash) :
size(size), num_hash(num_hash)
{
	sprintf(name, "Half CUSketch");

	if (size <= 0 || num_hash <= 0)
	{
		panic("SIZE & NUM_HASH must be POSITIVE integers.");
	}
	cnt = new int[size];
	fp = new int[size];
	prev = new LL[size];
	hash = new BOBHash32[num_hash];

	row_size = size / num_hash;
}

HCUSketch::~HCUSketch()
{
	if (cnt)
		delete [] cnt;
	if (fp)
		delete [] fp;
	if (prev)
		delete [] prev;
	if (hash)
		delete [] hash;
}

void
HCUSketch::init()
{
	memset(cnt, 0, size * sizeof(int));
	memset(fp, 0, size * sizeof(int));
	memset(prev, 0, size * sizeof(LL));

	for (int i = 0; i < num_hash; ++i)
	{
		hash[i].initialize(rand() % MAX_PRIME32);
	}
}

void
HCUSketch::status()
{
	printf("bucket: %d   hash: %d\n", size, num_hash);
}

void
HCUSketch::insert(int x, int v)
{
	while (v)
	{
		int minus = v;

		int minp = INT_MAX, secp = INT_MAX;
		int i = 0, base = 0;
		int sav_pos[10];

		for (i = 0, base = 0; i < num_hash; ++i, base += row_size)
		{
			int pos = hash[i].run((char*)&x, sizeof(int)) % row_size + base;
			sav_pos[i] = pos;
			if (minp >= cnt[pos])
			{
				minp = cnt[pos];
			}
			else
			{
				secp = min(secp, cnt[pos]);
				minus = min(minus, secp - minp);
			}
		}

		minp = INT_MAX;
		for (i = 0; i < num_hash; ++i)
		{
			int pos = sav_pos[i];
			minp = min(minp, cnt[pos]);
			if (cnt[pos] == minp)
				cnt[pos] += minus;
		}

		v -= minus;
	}
}

void
HCUSketch::insert_interval(int v, LL now)
{
	int i = 0, base = 0;

	for (i = 0, base = 0; i < num_hash; ++i, base += row_size)
	{
		int pos = hash[i].run((char*)&v, sizeof(int)) % row_size + base;
		if (prev[pos] != 0 && now - prev[pos] < 10000000)
		{
			LL interval = (now - prev[pos]) / 10;
			if (interval > INT_MAX)
			{
				printf("help!\n");
			}
			cnt[pos] = max(cnt[pos], int(interval));
			// printf("in! %d(%lld %lld)  ", cnt[pos], prev[pos], now);
		}
		if(now == 0)
			printf("!!!!!!!!!!!!!\n");
		prev[pos] = now;
	}
	// printf("\n");
}

int
HCUSketch::query_freq(int v)
{
	int ans = INT_MAX;
	int i = 0, base = 0;

	for (i = 0, base = 0; i < num_hash; ++i, base += row_size)
	{
		int pos = hash[i].run((char*)&v, sizeof(int)) % row_size + base;
		ans = min(ans, cnt[pos]);
	}

	return ans;
}

