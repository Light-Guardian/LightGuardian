#include <bits/stdc++.h>
#include <arpa/inet.h>

using namespace std;

const int SketchFragmentSize = 65536;
const int MacAddressSize = 6;
const int Ipv4HeaderSize = 20;
const int IpAddressSize = 4;
const int TcpHeaderSize = 20;
const int VXLanHeaderSize = 36;

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

struct SketchletHeader  {
	uint16_t type;
	uint16_t switchID;
	uint16_t fragmentID;
	uint16_t sketchID;

	uint8_t sketchFragment[16];

	void logged(_IO_FILE* fd) {
		fprintf(fd, "type: 0x%04x, switch id: %d, sketch id: %d, fragment id: %d.\n", 
				type, switchID, sketchID, fragmentID);
	}

	void ntoh() {
		type = ntohs(type);
		switchID = ntohs(switchID);
		fragmentID = ntohs(fragmentID);
		sketchID = ntohs(sketchID);
	}
};

#pragma pack(2)
struct PacketData {
	EthernetHeader ethernetHeader;
	SketchletHeader sketchletHeader;
	Ipv4Header ipv4Header;
	TcpHeader tcpHeader;
};
#pragma pack()

struct SketchFragment {
	double timestamp;
	uint64_t seqno;

	uint32_t switchID;
	uint32_t sketchID;
	uint32_t fragmentID;
};

struct EndHostInfo {
	uint32_t hostIP;
	string pcapFilename;
};

bool operator < (SketchFragment &fragmentA, SketchFragment &fragmentB) {
	return fragmentA.timestamp < fragmentB.timestamp;
}

vector<SketchFragment> fragmentPool;
vector<EndHostInfo> endHostPool;

unordered_map<uint32_t, int32_t> switchFragmentDistinct;
unordered_map<uint32_t, int32_t> switchFragmentTotal;
unordered_map<uint32_t, bool*> switchFragmentRepo;
unordered_map<uint32_t, double> switchFragmentRatio;
unordered_map<uint32_t, uint32_t> switchSketchID;

void PcapFileParser(EndHostInfo &endHostInfo) {
	ifstream pcap(endHostInfo.pcapFilename.c_str(), ios::binary | ios::in);

	fprintf(stdout, "End Host: 0x%x, Pcap File: %s.\n",
			endHostInfo.hostIP, endHostInfo.pcapFilename.c_str());

	GlobalHeader globalHeader;

	pcap.read((char *)&globalHeader, sizeof(GlobalHeader));

	PacketHeader packetHeader;
	PacketData packetData;

	int packetCounter = 0, fragmentCounter = 0;

	double totalRecvBytes = 0;
	double fragmentRecvBytes = 0;

	double initTimestamp = 0.0; 

	while (pcap.read((char *)&packetHeader, sizeof(PacketHeader))) {
		// packetHeader.logged(stdout);

		pcap.read((char *)&packetData, sizeof(PacketData));

		packetData.ethernetHeader.ntoh();
		// packetData.ethernetHeader.logged(stdout);
		packetData.sketchletHeader.ntoh();
		// packetData.sketchletHeader.logged(stdout);
		// packetData.ipv4Header.logged(stdout);

		pcap.seekg(packetHeader.incl_len - sizeof(PacketData), ios::cur);

		if (packetData.ethernetHeader.type != 0x1000) {
			continue;
		}
		if (packetData.ipv4Header.dstIP != endHostInfo.hostIP) {
			continue;
		}

		packetCounter += 1;
		totalRecvBytes += packetHeader.orig_len + VXLanHeaderSize;

		if (packetData.sketchletHeader.switchID != 0) {
			fragmentCounter += 1;
			fragmentRecvBytes += sizeof(SketchletHeader);

			SketchFragment fragment;

			fragment.timestamp = packetHeader.ts_sec + packetHeader.ts_usec * 0.000001;
			fragment.seqno = packetCounter;

			if (initTimestamp == 0.0) {
				initTimestamp = fragment.timestamp;
			}
			fragment.timestamp -= initTimestamp;

			fragment.sketchID = packetData.sketchletHeader.sketchID;
			fragment.switchID = packetData.sketchletHeader.switchID;
			fragment.fragmentID = packetData.sketchletHeader.fragmentID;

			fragmentPool.push_back(fragment);
		}
		else {
			totalRecvBytes -= sizeof(SketchletHeader);
		}

		if (packetCounter % 1000000 == 0) {
			fprintf(stdout, "read %dM packets.\n", packetCounter / 1000000);
		}
	}

	fprintf(stdout, "Total Packets: %d, Bandwidth Usage: %6fG.\n", 
			packetCounter, totalRecvBytes / 1024.0 / 1024.0 / 1024.0);
	fprintf(stdout, "Total Fragments: %d, Bandwidth Usage: %6fM.\n", 
			fragmentCounter, fragmentRecvBytes / 1024.0 / 1024.0);
	fprintf(stdout, "Fragments Ratio: %lf\n", 1.0 * fragmentCounter / packetCounter);
}

void PcapFileLoader() {
	ifstream pcapFile("pcap_config", ios::in);

	EndHostInfo endHostInfo;
	while (pcapFile >> hex >> endHostInfo.hostIP >> endHostInfo.pcapFilename) {
		PcapFileParser(endHostInfo);
	}
}

void FragmentsAnalyzer() {
	sort(fragmentPool.begin(), fragmentPool.end());

	for (auto fragment : fragmentPool) {
		if (switchFragmentTotal[fragment.switchID] == 0) {
			switchFragmentTotal[fragment.switchID] = 0;
			switchFragmentRepo[fragment.switchID] = new bool[SketchFragmentSize];

			switchSketchID[fragment.switchID] = 1;

			switchFragmentDistinct[fragment.switchID] = 0;
			switchFragmentRatio[fragment.switchID] = 0.0;

			memset(switchFragmentRepo[fragment.switchID], 0, SketchFragmentSize);
		}
		
		if (fragment.sketchID != switchSketchID[fragment.switchID]) {
			switchSketchID[fragment.switchID] = fragment.sketchID;

			switchFragmentDistinct[fragment.switchID] = 0;
			switchFragmentRatio[fragment.switchID] = 0.0;

			memset(switchFragmentRepo[fragment.switchID], 0, SketchFragmentSize);
		}

		switchFragmentTotal[fragment.switchID] += 1;
		if (!switchFragmentRepo[fragment.switchID][fragment.fragmentID]) {
			switchFragmentDistinct[fragment.switchID] += 1;
			switchFragmentRepo[fragment.switchID][fragment.fragmentID] = true;
		}

		if (switchFragmentDistinct[fragment.switchID] >= 
				SketchFragmentSize * switchFragmentRatio[fragment.switchID]) {
			fprintf(stdout, "[%10lf, %10d] switch %d collecting ... %.2lf %% fragments.\n",
					fragment.timestamp, fragment.seqno, fragment.switchID,
				       	switchFragmentRatio[fragment.switchID]);
			switchFragmentRatio[fragment.switchID] += 0.01;
		}
	}

	for (auto switchFragmentInfo : switchFragmentTotal) {
		fprintf(stdout, "Switch ID: %d, Total Fragments: %d, Distinct Fragments: %d.\n",
				switchFragmentInfo.first, switchFragmentInfo.second, 
				switchFragmentDistinct[switchFragmentInfo.first]);
	}
}

int main() {
	PcapFileLoader();
	
	FragmentsAnalyzer();

	return 0;
}
