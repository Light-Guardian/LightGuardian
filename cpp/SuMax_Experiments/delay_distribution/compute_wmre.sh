#!/bin/sh
g++ delay_union_wmre_vs_memory.cpp --std=c++11 -o union.out && ./union.out &
g++ delay_mixed_wmre_vs_memory.cpp --std=c++11 -o mixed.out && ./mixed.out &
g++ delay_chi2_wmre_vs_memory.cpp --std=c++11 -o chi2.out && ./chi2.out &
g++ delay_distribution_wmre_with_topk.cpp --std=c++11 -o topk.out && ./topk.out &
