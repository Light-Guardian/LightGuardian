#include "minheap.h"
#include "utils.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <climits>
#include <cstdint>
#include <map>
using std::map;

bool
MinHeap::insert(int fp, int v)
{
	// existence
	bool flag = false;
	{
		set<PII>::iterator it = rdx.lower_bound(mp(fp, 0));
		if (it != rdx.end() && (*it).ft == fp)
		{
			int old_v = (*it).sc;
			rdx.erase(it);
			data.erase(data.lower_bound(mp(old_v, fp)));
			flag = true;
		}
	}


	if (data.size() < max_size)
	{
		rdx.insert(mp(fp, v));
		data.insert(mp(v, fp));
		return true;
	}

	set<PII>::iterator min_ele = data.begin();
	if((*min_ele).ft < v)
	{
		int old_fp = (*min_ele).sc;
		int old_v = (*min_ele).ft;
		rdx.erase(rdx.lower_bound(mp(old_fp, old_v)));
		data.erase(min_ele);

		rdx.insert(mp(fp, v));
		data.insert(mp(v, fp));
		return true;
	}

	return false;
}

vector<PII>
MinHeap::topk(int k)
{
	set<PII>::iterator it = data.end();
	vector<PII> ans;

	if (data.size() < k)
		k = (int)data.size();
	while (k--)
	{
		it--;
		ans.push_back((*it));
	}
	// reverse(ans.begin(), ans.end());
	
	return ans;
}