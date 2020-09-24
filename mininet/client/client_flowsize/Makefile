all : sender receiver traffic


traffic : traffic.cpp sender receiver
	g++ traffic.cpp -o traffic -O2 --std=c++11 -lpthread -g

sender : sender.cpp sender.h host.h
	g++ sender.cpp -o sender -O2 --std=c++11 -g

receiver : receiver.cpp receiver.h aggregator.h host.h
	g++ receiver.cpp -o receiver -O2 --std=c++11 -g

clean :
	rm sender receiver
