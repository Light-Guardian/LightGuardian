#ifndef __RECEIVER_H__
#define __RECEIVER_H__

#include "header.h"
#include "host.h"
#include "aggregator.h"

namespace Simulator {

struct Receiver : Host {
	sockaddr_in addr_send, addr_recv;
	COM_Header com_header;

	int sock_fd, recv_port, addr_len, header_len;

	Aggregator *aggregator;

	Receiver(int port, Aggregator *agg, std::string pid) : 
		Host(pid), aggregator(agg) {
		sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

		if (sock_fd < 0) {
			perror("socket.");
			exit(1);
		}

		recv_port = port;

		// printf("port: %d.\n", recv_port);

		addr_len = sizeof(addr_recv);
		header_len = sizeof(com_header);

		memset((char *)&addr_send, 0x00, addr_len);
		memset((char *)&addr_recv, 0x00, addr_len);
		memset((char *)&com_header, 0x00, header_len);

		addr_recv.sin_family = AF_INET;
		addr_recv.sin_addr.s_addr = htonl(INADDR_ANY);
		addr_recv.sin_port = htons(recv_port);

		if (bind(sock_fd, (sockaddr *)&addr_recv, addr_len) < 0) {
			perror("bind error.");
			exit(1);
		}
	}

	void receive(int maxp) {
		for (int recvp = 0; recvp < maxp || maxp < 0; recvp++) {
			int recv_num = recvfrom(sock_fd, (char *)&com_header, header_len,
				   	0, (sockaddr *)&addr_send, (socklen_t *)&addr_len);

			// printf("receive: %d / %d bytes (PKT : %d, FG : %d)\n", recv_num,
			// 	   	header_len, recvp, com_header.mih.exists_fg);
			// printf("send ip: %s, send port: %d.\n",
			// 	   	inet_ntoa(addr_send.sin_addr), addr_send.sin_port);

			if (recv_num < 0) {
				perror("recvfrom error.");
				exit(1);
			}

			betoh_header(com_header);
			if (com_header.flag_header.exists_fg == 2 || com_header.flag_header.exists_fg == 3) {
				aggregator->receive_header(com_header);
			}
		}
	}

	void betoh_header(COM_Header &com_header) {
		
		com_header.sfh.switch_id = be16toh(com_header.sfh.switch_id);
		com_header.sfh.fgment_id = be32toh(com_header.sfh.fgment_id);

		for (int i = 0; i < 8; i++) {
			com_header.sfh.delay[i] = be32toh(com_header.sfh.delay[i]);
		}
	}

	~Receiver() {
		close(sock_fd);
	}
};
}

#endif
