#ifndef __HOST_H__
#define __HOST_H__

#include <cstdlib>
#include <cstring>
#include <chrono>
#include <thread>
#include <iostream>
#include <fstream>
#include <mutex>

#include <unistd.h>
#include <syscall.h>
#include <fcntl.h>
#include <sched.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "header.h"

namespace Simulator {

struct Host {
	Host(std::string pid) {
		switch_namespace(pid);
	}

	void switch_namespace(std::string pid) {
		char path[MAX_PATH_LENGTH];

		sprintf(path, "/proc/%s/ns/net", pid.c_str());
		int status_net = attachToNS(path);
		// printf("attach net: %d.\n", status_net);

		sprintf(path, "/proc/%s/ns/pid", pid.c_str());
		int status_pid = attachToNS(path);
		// printf("attach pid: %d.\n", status_pid);

		sprintf(path, "/proc/%s/ns/mnt", pid.c_str());
		int status_mnt = attachToNS(path);
		// printf("attach mnt: %d.\n", status_mnt);

		// if (system("ifconfig | grep inet") != 0) {
		// 	exit(0);
		// }
	}

	int attachToNS(char *path) {
		int nsid = open(path, O_RDONLY);
		return setns(nsid, 0);
	}

};

}

#endif
