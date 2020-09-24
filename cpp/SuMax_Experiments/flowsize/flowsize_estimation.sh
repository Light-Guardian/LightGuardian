#!/bin/sh
g++ flowsize_ARE_vs_memory.cpp --std=c++11 -o flow.out && ./flow.out &
g++ card_dist_ent_vs_memory.cpp --std=c++11 -o cde.out && ./cde.out & 