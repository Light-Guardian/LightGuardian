all : sender receiver traffic


traffic : traffic.cpp sender.h receiver.h
	g++ traffic.cpp -o traffic -O2 --std=c++11 -lpthread -g
