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

map<string, string> hostPID, hostIP;
map<string, thread*> hostThread;
map<string, mutex*> hostMutex;

vector<thread*> connThread;

void ThreadSend(string sname, string rname, int maxp) {
	Simulator::Sender sender(hostIP[sname], hostIP[rname], 50806, hostPID[sname]);

	sender.set_mutex(hostMutex[sname]);
	sender.send(maxp);
}

void ThreadRecv(string rname) {
	Simulator::Receiver receiver(hostIP[rname], 50806, nullptr, hostPID[rname]);

	receiver.receive(-1);
}

void EndhostLoader() {
	if (system("./config.sh") != 0) {
		exit(0);
	}

	ifstream loader("host.config", ios::in);

	string rname, pid, ip;

	while (loader >> rname >> pid >> ip) {
		hostPID[rname] = pid;
		hostIP[rname] = ip;
		hostThread[rname] = new thread(ThreadRecv, rname);
		hostMutex[rname] = new mutex;
	}
}

void TrafficLoader() {
	ifstream loader("traffic.config", ios::in);

	string sname, rname;
	int maxp;

	while (loader >> sname >> rname >> maxp) {
		thread *conn = new thread(ThreadSend, sname, rname, maxp);
		connThread.push_back(conn);

		cout << sname << " => " << rname << " : " << maxp << endl;
	}

	for (auto conn : connThread) {
		conn->join();
	}
}

int main(int argc, char **argv) {
	EndhostLoader();

	TrafficLoader();

	std::this_thread::sleep_for(std::chrono::seconds(3600));

	return 0;
}
