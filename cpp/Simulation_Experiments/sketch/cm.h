#ifndef CMS_HEADER
#define CMS_HEADER

#include "sketch.h"
#include "minheap.h"
#include "BOBHash32.h"

class CMSketch : public Sketch
{
private:
	int size, num_hash, row_size;

	int *cnt, *fp;
	BOBHash32 *hash;

	MinHeap hp;

public:
	CMSketch(int size, int num_hash);
	~CMSketch();
	void init();
	void insert(int x, int v);
	int query_freq(int v);
	vector<PII> query_heavyhitter(int threshold);
	vector<PII> query_topk(int k);
	void status();
	void query_distribution(vector<double>& ans);
};

#endif