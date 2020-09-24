#ifndef MINHEAP_HEADER
#define MINHEAP_HEADER

#include "sketch.h"
#include <set>
using std::set;

class MinHeap
{
private:
	set<PII> data, rdx;
	int max_size;
public:
	MinHeap() { max_size = 300; };
	MinHeap(int x) : max_size(x) {};
	~MinHeap() {};
	int size() { return data.size(); }
	void init() { data.clear(); rdx.clear(); }
	void set_maxsize(int x) { max_size = x; }
	int get_maxsize() { return max_size; }
	PII top() { return data.size()==0? mp(0, 0):(*data.begin()); }
	bool insert(int fp, int v); // try to push, return 1 if success. 
	vector<PII> topk(int k); // return topk of the heap
};

#endif
