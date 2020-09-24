#ifndef __SENDER_H__
#define __SENDER_H__

#include "header.h"
#include "host.h" 

namespace Simulator {

struct Sender : Host {
	std::string send_ip, recv_ip;

	sockaddr_in addr_recv;
	SEND_Header send_header;

	int recv_port, sock_fd, addr_len, header_len;

	std::mutex *udp_mutex;

	Sender(std::string sip, std::string rip, int port, std::string pid) : Host(pid) {

		sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

		if (sock_fd < 0) {
			perror("socket error.");
			exit(1);
		}

		int nSendBuf = 128 * 1024 * 1024;
		setsockopt(sock_fd, SOL_SOCKET, SO_SNDBUF, (const char*) &nSendBuf, sizeof(int));

		send_ip = sip;
		recv_ip = rip;
		recv_port = port;

		// printf("ip: %s, port: %d\n", recv_ip.c_str(), recv_port);

		addr_len = sizeof(addr_recv);
		header_len = sizeof(send_header);

		memset((char *)&addr_recv, 0x00, addr_len);
		memset((char *)&send_header.load_header, 0xFF, sizeof(send_header.load_header));

		addr_recv.sin_family = AF_INET;
		addr_recv.sin_addr.s_addr = inet_addr(recv_ip.c_str());
		addr_recv.sin_port = htons(recv_port);

		send_header.flag_header.exists_fg = 0; 
	}

	void send(int maxp) {
		for (int sendp = 0; sendp < maxp || maxp < 0; sendp++) {
			udp_mutex->lock();

			send_header.load_header.packet_id += 1;

			int send_num = sendto(sock_fd, (char *)&send_header, header_len,
				   	0, (sockaddr *)&addr_recv, addr_len);

			// printf("send: %d / %d bytes (PKT : %d)\n", send_num, header_len, sendp);
			// printf("recv ip: %s, recv port: %d\n", recv_ip.c_str(), recv_port);

			printf("send (%6d, %6d) : %s -> %s.\n", sendp, send_header.load_header.packet_id,
					send_ip.c_str(), recv_ip.c_str());

			if (send_num < 0) {
				perror("sendto error.");
				exit(1);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(80));

			udp_mutex->unlock();
		}
	}

	void set_mutex(std::mutex *udp_mutex) {
		this->udp_mutex = udp_mutex;
	}

	~Sender() {
		close(sock_fd);
	}
};
}

#endif
