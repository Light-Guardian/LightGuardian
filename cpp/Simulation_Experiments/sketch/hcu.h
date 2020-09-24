#ifndef HCUS_HEADER
#define HCUS_HEADER

#include "sketch.h"
#include "BOBHash32.h"

class HCUSketch : public Sketch
{
private:
	int size, num_hash, row_size;

	int *cnt, *fp;
	LL *prev;
	BOBHash32 *hash;

public:
	HCUSketch(int size, int num_hash);
	~HCUSketch();
	void init();
	void insert(int x, int v);
	void insert_interval(int x, LL now);
	int query_freq(int v);
	void status();
};

#endif