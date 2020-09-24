#ifndef Param
#define Param
#include<bits/stdc++.h>
#include"BOBHash32.h"

#define fi first
#define se second
#define rep(i,a,b) for(int (i)=(a);(i)<=(b);(i)++)
#define rep2(i,a,b) for(int (i)=(a);(i)<(b);(i)++)
#define END_FILE_NO 1
#define START_FILE_NO 0
#define KEY_LENGTH 8
#define ITEM_LENGTH 16
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))
#define ABS(a) ((a)>0?(a):(-(a)))

#define Lev_Num 8
//#define Row_Num 3

#define ChangeBuckNum
#ifndef ChangeBuckNum
#define Buck_Num_PerRow 100000
#else
int Buck_Num_PerRow = 65536;
int Row_Num = 3;
double collect_rate = 1.0;
#endif

#define small
//#define lab

// path of CAIDA2018 dataset
#define datapath "/usr/share/dataset/CAIDA2018/dataset/130000.dat"

typedef __uint128_t flow_t;
typedef double_t timestamp_t;
typedef map<__uint128_t, vector<unsigned> > gtmap;
__uint128_t mistake = 0;
unsigned delayLevThres[Lev_Num] = {10, 20, 30, 40, 50, 60, 70, 80};
unsigned Zeros[Lev_Num] = {0, 0, 0, 0, 0, 0, 0, 0};
#endif