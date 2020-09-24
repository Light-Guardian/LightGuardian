#include"param.h"

typedef vector<pair<flow_t, double> > packet_in;
typedef vector<pair< vector<int>, packet_in>> packet_full;
double get_delay(){
    int tmp = rand() % 10000;
    return 0.1;
    return tmp*0.000001;
}
bool Comp_pair(const tuple<flow_t, double, int> & a, const tuple<flow_t, double, int> & b){
    return get<1>(a) < get<1>(b);
}
bool Comp(const pair<flow_t, unsigned> &a, const pair<flow_t, unsigned> &b){
    return a.second > b.second;
}