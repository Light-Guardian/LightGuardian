#ifndef CUS_HEADER
#define CUS_HEADER

#include "sketch.h"
#include "BOBHash32.h"

class CUSketch : public Sketch
{
private:
	int size, num_hash, row_size;

	int *cnt, *fp;
	BOBHash32 *hash;

public:
	CUSketch(int size, int num_hash);
	~CUSketch();
	void init();
	void insert(int x, int v);
	int query_freq(int v);
	void status();
	CUSketch* merge(CUSketch *q);
};

#endif