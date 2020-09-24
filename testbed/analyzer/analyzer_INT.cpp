#include <bits/stdc++.h>
#include <arpa/inet.h>

using namespace std;

const int MacAddressSize = 6;
const int Ipv4HeaderSize = 20;
const int IpAddressSize = 4;
const int TcpHeaderSize = 20;
const int VXLanHeaderSize = 50;

struct GlobalHeader {
	uint32_t magic_number;
	uint16_t version_major;
	uint16_t version_minor;
	int32_t thiszone;
	uint32_t sigfigs;
	uint32_t snaplen;
	uint32_t network;
};

struct PacketHeader {
	uint32_t ts_sec;
	uint32_t ts_usec;
	uint32_t incl_len;
	uint32_t orig_len;

	void logged(_IO_FILE* fd) {
		fprintf(fd, "timestamp: %d.%06d, incl length: %d, orig length: %d\n", 
				ts_sec, ts_usec, incl_len, orig_len);
	}
};

struct EthernetHeader {
	uint8_t srcMac[MacAddressSize];
	uint8_t dstMac[MacAddressSize];
	uint16_t type;

	void logged(_IO_FILE* fd) {
		fprintf(fd, "type: 0x%04x.\n", type);
	}

	void ntoh() {
		type = ntohs(type);
	}
};

struct Ipv4Header {
	uint8_t ipv4Header1[Ipv4HeaderSize - IpAddressSize * 2];
	uint32_t srcIP;
	uint32_t dstIP;

	void logged(_IO_FILE* fd) {
		uint8_t *srcIpBytes = (uint8_t *)&srcIP;
		uint8_t *dstIpBytes = (uint8_t *)&dstIP;
		fprintf(fd, "%p, %p, %p.\n", this, srcIpBytes, dstIpBytes);
		fprintf(fd, "src IP: %u.%u.%u.%u, dst IP: %u.%u.%u.%u, (0x%x, 0x%x)\n",
				srcIpBytes[0], srcIpBytes[1], srcIpBytes[2], srcIpBytes[3],
				dstIpBytes[0], dstIpBytes[1], dstIpBytes[2], dstIpBytes[3],
				srcIP, dstIP);
	}
};

struct TcpHeader {
	uint8_t tcpHeader[TcpHeaderSize];
};

struct IntHeader  {
	uint8_t TF;
	uint8_t count;
	uint16_t type;

	uint8_t sketchInt[100];

	void logged(_IO_FILE* fd) {
		fprintf(fd, "type: 0x%04x, count: %d.\n", type, count);
	}

	void ntoh() {
		type = ntohs(type);
	}
};

#pragma pack(2)
struct PacketData {
	EthernetHeader ethernetHeader;
	IntHeader intHeader;
	Ipv4Header ipv4Header;
	TcpHeader tcpHeader;
};
#pragma pack()

struct EndHostInfo {
	uint32_t hostIP;
	string pcapFilename;
};

void PcapFileParser(EndHostInfo &endHostInfo) {
	ifstream pcap(endHostInfo.pcapFilename.c_str(), ios::binary | ios::in);

	fprintf(stdout, "End Host: 0x%x, Pcap File: %s.\n",
			endHostInfo.hostIP, endHostInfo.pcapFilename.c_str());

	GlobalHeader globalHeader;

	pcap.read((char *)&globalHeader, sizeof(GlobalHeader));

	PacketHeader packetHeader;
	PacketData packetData;

	int packetCounter = 0, intCounter = 0;

	double totalRecvBytes = 0;
	double intRecvBytes = 0;

	double initTimestamp = 0.0; 

	while (pcap.read((char *)&packetHeader, sizeof(PacketHeader))) {
		// packetHeader.logged(stdout);

		pcap.read((char *)&packetData, sizeof(PacketData));

		packetData.ethernetHeader.ntoh();
		// packetData.ethernetHeader.logged(stdout);
		packetData.intHeader.ntoh();
		packetData.intHeader.logged(stdout);
		packetData.ipv4Header.logged(stdout);

		pcap.seekg(packetHeader.incl_len - sizeof(PacketData), ios::cur);

		// if (packetData.ethernetHeader.type != 0x1000) {
		// 	continue;
		// }
		// if (packetData.ipv4Header.dstIP != endHostInfo.hostIP) {
		// 	continue;
		// }

		packetCounter += 1;
		totalRecvBytes += packetHeader.orig_len + VXLanHeaderSize - sizeof(IntHeader);

		intRecvBytes += 4 + 20 * packetData.intHeader.count;
		totalRecvBytes += 4 + 20 * packetData.intHeader.count;

		if (packetCounter % 1000000 == 0) {
			fprintf(stdout, "read %dM packets.\n", packetCounter / 1000000);
		}
		if (packetCounter >= 20) {
			break;
		}
	}

	fprintf(stdout, "Total Packets: %d, Bandwidth Usage: %6fG.\n", 
			packetCounter, totalRecvBytes / 1024.0 / 1024.0 / 1024.0);
	fprintf(stdout, "Total Ints: %d, Bandwidth Usage: %6fM.\n", 
			intCounter, intRecvBytes / 1024.0 / 1024.0);
}

void PcapFileLoader() {
	ifstream pcapFile("pcap_config", ios::in);

	EndHostInfo endHostInfo;
	while (pcapFile >> hex >> endHostInfo.hostIP >> endHostInfo.pcapFilename) {
		PcapFileParser(endHostInfo);
	}
}

int main() {
	PcapFileLoader();

	return 0;
}
