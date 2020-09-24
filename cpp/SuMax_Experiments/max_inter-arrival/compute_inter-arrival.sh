#!/bin/sh
g++ max_interval_vs_memory.cpp --std=c++11 -o maxint_mem.out && ./maxint_mem.out &
g++ max_interval_vs_topk.cpp --std=c++11 -o maxint_topk.out && ./maxint_topk.out &