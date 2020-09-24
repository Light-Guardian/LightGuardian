//#include"param.h"
#include<random>
#include <chrono>
unsigned get_ChiSquare(int n, int scale){
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine gen(seed);
    std::normal_distribution<double> dis(0,1);
    double result = 0;
    for(int i = 0; i < n; i++){
        double tmp = (double)scale * dis(gen);
        result += tmp * tmp;
    }
    return (unsigned)round(result);
}

unsigned get_Union(int a, int b){
    int Range = b - a;
    return a + (rand() % Range);
}