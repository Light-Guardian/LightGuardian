#include "cu.h"
#include "utils.h"
#include <cstdio>
#include <cmath>
#include <cstring>
#include <climits>

CUSketch::CUSketch(int size, int num_hash) :
size(size), num_hash(num_hash)
{
	sprintf(name, "CUSketch");

	if (size <= 0 || num_hash <= 0)
	{
		panic("SIZE & NUM_HASH must be POSITIVE integers.");
	}
	cnt = new int[size];
	fp = new int[size];
	hash = new BOBHash32[num_hash];

	row_size = size / num_hash;
}

CUSketch::~CUSketch()
{
	if (cnt)
		delete [] cnt;
	if (fp)
		delete [] fp;
	if (hash)
		delete [] hash;
}

void
CUSketch::init()
{
	memset(cnt, 0, size * sizeof(int));
	memset(fp, 0, size * sizeof(int));

	for (int i = 0; i < num_hash; ++i)
	{
		hash[i].initialize(rand() % MAX_PRIME32);
	}
}

void
CUSketch::status()
{
	printf("bucket: %d   hash: %d\n", size, num_hash);
}

void
CUSketch::insert(int x, int v)
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
			minp = min(minp, cnt[pos]);
		}

		for (i = 0; i < num_hash; ++i)
		{
			int pos = sav_pos[i];
			if (cnt[pos] != minp)
				secp = min(secp, cnt[pos]);
		}

		minus = min(minus, secp - minp);

		for (i = 0, base = 0; i < num_hash; ++i, base += row_size)
		{
			int pos = sav_pos[i];
			// int pos = hash[i].run((char*)&v, sizeof(int)) % row_size + base;
			if (cnt[pos] == minp)
				cnt[pos] += minus;
		}

		v -= minus;
	}
}

int
CUSketch::query_freq(int v)
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

CUSketch*
CUSketch::merge(CUSketch *q)
{
	if (size != q->size || num_hash != q->num_hash)
	{
		printf("error! size must be equal.\n");
		return NULL;
	}

	CUSketch * mrg = new CUSketch(size, num_hash);
	mrg->init();

	int base, i;
	for (i = 0, base = 0; i < num_hash; ++i, base += row_size)
	{
		for (int j = 0; j < row_size; ++j)
		{
			int pos = base + j;
			mrg->cnt[pos] = cnt[pos] + q->cnt[pos];
		}
	}

	return mrg;
}
