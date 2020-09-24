#ifndef IMPL_HEADER
#define IMPL_HEADER

#include <iostream>
#include <vector>
using std::vector;
using std::pair;

typedef pair<int, int> PII;
typedef long long LL;
#define mp std::make_pair
#define ft first
#define sc second

class Sketch
{
public:
	char name[100];

	virtual void init()
	{
		std::cout << "No init function." << std::endl;
	}

	virtual void insert(int x, int v)
	{
		std::cout << "No insert function." << std::endl;
	}

	virtual void insert_interval(int x, LL now)
	{
		std::cout << "No insert function." << std::endl;
	}

	virtual void remove()
	{
		std::cout << "No remove function." << std::endl;
	}

	virtual bool query_exist(int x)
	{
		std::cout << "No query function." << std::endl;
	}

	virtual int query_max(int x)
	{
		std::cout << "No query function." << std::endl;
	}

	virtual int query_freq(int x)
	{
		std::cout << "No query function." << std::endl;
	}

	virtual vector<PII> query_topk(int k)
	{
		std::cout << "No query function." << std::endl;
	}

	virtual vector<PII> query_heavyhitter(int threshold)
	{
		std::cout << "No query function." << std::endl;
	}

	virtual void query_distribution(vector<double>& ans)
	{
		std::cout << "No query function." << std::endl;
	}

	virtual void status()
	{
		std::cout << "No status function." << std::endl;
	}
};

#endif