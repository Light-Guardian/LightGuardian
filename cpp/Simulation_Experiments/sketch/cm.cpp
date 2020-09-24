#include "cm.h"
#include "utils.h"
#include "EMSFD.h"
#include <cstdio>
#include <cmath>
#include <cstring>
#include <climits>

CMSketch::CMSketch(int size, int num_hash) :
size(size), num_hash(num_hash)
{
	sprintf(name, "CMSketch");

	if (size <= 0 || num_hash <= 0)
	{
		panic("SIZE & NUM_HASH must be POSITIVE integers.");
	}
	cnt = new int[size];
	fp = new int[size];
	hash = new BOBHash32[num_hash];
	
	row_size = size / num_hash;
}

CMSketch::~CMSketch()
{
	if (cnt)
		delete [] cnt;
	if (fp)
		delete [] fp;
	if (hash)
		delete [] hash;
}

void
CMSketch::init()
{
	memset(cnt, 0, size * sizeof(int));
	memset(fp, 0, size * sizeof(int));

	for (int i = 0; i < num_hash; ++i)
	{
		hash[i].initialize(rand() % MAX_PRIME32);
	}
}

void
CMSketch::status()
{
	printf("bucket: %d   hash: %d\n", size, num_hash);
}

void
CMSketch::insert(int x, int v)
{
	int i = 0, base = 0;

	for (i = 0, base = 0; i < num_hash; ++i, base += row_size)
	{
		int pos = hash[i].run((char*)&x, sizeof(int)) % row_size + base;
		cnt[pos] += v;
	}
}

int
CMSketch::query_freq(int v)
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

vector<PII>
CMSketch::query_heavyhitter(int threshold)
{
	vector<PII> tmp, ans;

	tmp = hp.topk(hp.get_maxsize());

	for (int i = 0; i < tmp.size(); ++i)
	{
		printf("%d %d\n", tmp[i].first, tmp[i].second);
	}

	int sz = tmp.size();
	for (int i = 0; i < sz; ++i)
	{
		if (tmp[i].first < threshold) break;
		ans.push_back(tmp[i]);
	}

	return ans;
}

vector<PII>
CMSketch::query_topk(int k)
{
	return hp.topk(k);
}

void
CMSketch::query_distribution(vector<double>& ans)
{
	if (num_hash != 1)
	{
		printf("The answer may be wrong because NUM_HASH is not equal to 1.\n");
	}

	uint32_t *tmp_cnt;
	tmp_cnt = new uint32_t[size];

	for (int i = 0; i < size; ++i)
		tmp_cnt[i] = cnt[i];

	EMFSD *em_fsd_algo;
	em_fsd_algo = new EMFSD();
    em_fsd_algo->set_counters(size, tmp_cnt);

    for (int i = 0; i < 10; ++i)
    {
    	printf("epoch %d\n", i);
    	em_fsd_algo->next_epoch();
    }

    delete [] tmp_cnt;

    ans = em_fsd_algo->ns;
}
