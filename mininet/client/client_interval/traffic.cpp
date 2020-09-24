#include <map>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>

#include <unistd.h>
#include <syscall.h>
#include <fcntl.h>
#include <sched.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cmath>
#include <mutex>
#include <thread>

#include "header.h"
#include "sender.h"
#include "receiver.h"
#include "aggregator.h"

using namespace std;

string hname[MAX_HOST_NUM];
map<string, string> host_pid, host_ip;

std::mutex my_mutex;
vector<int> udp_port_array;
float lambda = 0.001;

int flow_size_generator(){
    float prob_array[9] = {0.0, 0.5, 0.6, 0.7, 0.8, 0.9, 0.95, 0.99, 1.0};
    int flow_size_array[9] = {100, 1000, 3000, 4000, 10000, 400000, 3000000, 90000000, 900000000};
    
    float prob = (float)(rand() % 101) / 100;
    int index;

    if (prob == prob_array[0]){
        return flow_size_array[0];
    }else{
        for (int i = 1; i < 9; ++i){
            if (prob <= prob_array[i]){
                index = i;
                break;
            }
        }

		return (int)(flow_size_array[index] - (prob_array[index] - prob)/(prob_array[index] - prob_array[index-1])*(flow_size_array[index] - flow_size_array[index-1]));
    }

}

int flow_interval_generator(double lambda){
	double pV = 0.0;
    while(true)
    {
        pV = (double)rand()/(double)RAND_MAX;
        if (pV != 1)
        {
            break;
        }
    }
    pV = (-1.0/lambda)*log(1-pV);
    return (int)pV;
}

void Connector_S(string sname, string rname, int rport, int maxp) {
	Simulator::Sender sender(host_ip[rname], rport, host_pid[sname]);

	sender.send(maxp);
}

void Connector_R(string rname, int rport, int maxp, 
		Simulator::Aggregator *agg) {
	Simulator::Receiver receiver(rport, agg, host_pid[rname]);

	receiver.receive(maxp);
}

void Connector_pair(std::thread **receiver, std::thread **sender, string sname, string rname, Simulator::Aggregator *agg, int &current_conn_num, int conn_num){
	
	while (true)
	{
		my_mutex.lock();

		if (current_conn_num >= conn_num) {
			my_mutex.unlock();
			break;
		}

		int udp_port_index = rand() % udp_port_array.size();
		int rport = udp_port_array[udp_port_index];
		udp_port_array.erase(udp_port_array.begin() + udp_port_index, udp_port_array.begin() + udp_port_index + 1);
		
		receiver[current_conn_num] = new std::thread(Connector_R, rname, rport, -1, agg);

		int sendp = flow_size_generator();
		int iterv = flow_interval_generator(lambda);
		sender[current_conn_num] = new std::thread(Connector_S, sname, rname, rport, sendp);

		std::cout << sname << " => " << rname << ":" << rport << "(" << sendp << ")" << std::endl;

		ofstream out;
		out.open("flow.txt", ios::out | ios::app);
		if (out.is_open())
		{
			out << sname << " => " << rname << ":" << rport << "(" << sendp << ")" << '\n';
		}
		out.close();

		current_conn_num++;

		my_mutex.unlock();

		std::this_thread::sleep_for(std::chrono::milliseconds(iterv));
	}
	
}

int config_loader() {
	if (system("./config.sh") != 0) {
		exit(0);
	}

	ifstream loader("./host.config", std::ios::in);

	int host_num = 0;
	string name, pid, ip;

	while (loader >> hname[host_num] >> pid >> ip) {
		host_pid[hname[host_num]] = pid;
		host_ip[hname[host_num]] = ip;
		host_num += 1;
	}

	std::cout << "hosts:" << std:: endl;
	for (int i = 0; i < host_num; i++) {
		std::cout << hname[i] << " : (" << host_pid[hname[i]] << ", " <<
		   	host_ip[hname[i]] << ")" << std::endl;
	}

	return host_num;
}

void traffic_generator(int host_num, int conn_num) {
	Simulator::Aggregator aggregator(Simulator::sw_num, Simulator::ar_num, 
			Simulator::bu_num);

	std::thread **receiver = new std::thread*[conn_num];
	std::thread **sender = new std::thread*[conn_num]; 
	std::thread **pair = new std::thread*[host_num*(host_num - 1)];

	int pair_index = 0;
	int current_conn_num = 0;
	
	ofstream out;
        out.open("flow.txt", ios::out | ios::trunc);
        out.close();

	for (int i = 0; i < host_num; i++) {
		for (int j = 0; j < host_num; j++) {
			if (j != i) {
				string sname = hname[i];
				string rname = hname[j];

				pair[pair_index++] = new thread(Connector_pair, receiver, sender, sname, rname,
					   	&aggregator, std::ref(current_conn_num), conn_num);
			}
		}
	}
	
	for (int i = 0; i < host_num * (host_num - 1); i++)
	{
		pair[i] -> join();
	}

	for (int i = 0; i < conn_num; i++)
	{
		receiver[i] -> join();
	}

	for (int i = 0; i < conn_num; i++)
	{
		sender[i] -> join();
	}
}

int main(int argc, char **argv) {
	if (argc != 2) {
		perror("args error.");
		exit(1);
	}

	int host_num = config_loader();
	int conn_num = atoi(argv[1]); 
	int count = 0;

	while (count < conn_num)
	{
		int udp_port = rand() % 2000 + 20000;
		if (find(udp_port_array.begin(), udp_port_array.end(), udp_port) == udp_port_array.end())
		{
			udp_port_array.push_back(udp_port);
			count++;
		}
		
	}

	traffic_generator(host_num, conn_num);

	return 0;
}
