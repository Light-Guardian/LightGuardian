#ifndef __AGGREGATOR_H__
#define __AGGREGATOR_H__

#include <cstdlib>
#include <cstring>
#include <chrono>
#include <thread>
#include <string>
#include <iostream>
#include <fstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "header.h"

using namespace std;

namespace Simulator {

using namespace std::chrono;

struct Sketch {
	int ar_num, bu_num; //array_num,bucket_num

	MIH_Header **sketch;

	int switch_id, sketch_fg, fgment_nu; //sketch_flag,fragment_num

	steady_clock::time_point time_beg;
	steady_clock::time_point time_end;

	Sketch(int sid, int anum, int bnum) {
		ar_num = anum;
		bu_num = bnum;

		sketch = new MIH_Header*[ar_num];
		for (int i = 0; i < ar_num; i++) {
			sketch[i] = new MIH_Header[bu_num];
			memset(sketch[i], 0xFF, bu_num * sizeof(MIH_Header));
		}

		switch_id = sid;
		sketch_fg = 0;
		fgment_nu = 0;
	}

	void receive_fgment(int sfg, int fid, MIH_Header &mih_header) {
		int aid = mih_header.fgment_id / bu_num;
		int bid = mih_header.fgment_id % bu_num;

		if (sfg != sketch_fg) { 
			recv_next_sketch(sfg);
		}

		// printf("switch: %d, array: %d, bucket: %d.\n", switch_id, aid, bid);

		if (sketch[aid][bid].switch_id == 0xFFFF) {
			recv_next_fgment(mih_header, aid, bid);
		}
	}

	void recv_next_sketch(int sfg) {
		printf("switch %d detect sketch %d.\n", switch_id, sfg);

		sketch_fg = sfg;
		fgment_nu = 0;
		for (int i = 0; i < ar_num; i++) {
			memset(sketch[i], 0xFF, bu_num * sizeof(MIH_Header));
		}

		time_beg = steady_clock::now();
	}

	void recv_next_fgment(MIH_Header &mih_header, int aid, int bid) {
		sketch[aid][bid] = mih_header;
		fgment_nu += 1;

		printf("switch %d detect fgment %d (%d / %d).\n", 
				switch_id, mih_header.fgment_id, fgment_nu, bu_num * ar_num);

		if (fgment_nu == bu_num * ar_num) {
			recv_full_sketch();
		}
	}

	void recv_full_sketch() {
		time_end = steady_clock::now();

		auto time_epoch = duration_cast<milliseconds> (time_end - time_beg);
		printf("switch %d receive sketch %d using %ld ms.\n",
			   	switch_id, sketch_fg, time_epoch.count());

		download_sketch();
	}
	/*
	void visor_sketch() {
		for (int i = 0; i < ar_num; i++) {
			printf("sketch array %d.\n", i);
			for (int j = 0; j < bu_num; j++) {
				printf("|%3u|", sketch[i][j].timestamp);
				printf("\n");
			}
		}
	}*/
	
	void download_sketch(){
		ofstream out;
		string filename = "./download_sketch/switch" + to_string(switch_id) + ".txt"; 
		out.open(filename, ios::out | ios::trunc);
		if (!out.is_open())
		{
			return;
		} else
		{
			for (int i = 0; i < ar_num; i++)
			{
				for (int j = 0; j < bu_num; j++)
				{
					out << sketch[i][j].timestamp <<'\t';
				}

				out << '\n';
			}

		}
		
		out.close();

	}

};

struct Aggregator {
	int sw_num, ar_num, bu_num;

	Sketch **sketch;

	Aggregator(int snum, int anum, int bnum) {
		sw_num = snum;
		ar_num = anum;
		bu_num = bnum;

		sketch = new Sketch*[sw_num];
		for (int i = 0; i < sw_num; i++) {
			sketch[i] = new Sketch(i, anum, bnum);
		}
	}

	void display_header(COM_Header &com_header) {
		
		printf("mih.switch_id : %u\n" , com_header.mih.switch_id);
		printf("mih.sketch_fg : %u\n" , com_header.flag_header.exists_fg & 0b100);
		printf("mih.fgment_id : %u\n" , com_header.mih.fgment_id);

		printf("sketch fragment :\n");	
		printf("mih.timestamp : %lu\n", com_header.mih.timestamp);
		
		printf("\n");
	}

	void receive_header(COM_Header &com_header) {
		int switch_id = com_header.mih.switch_id;
		int sketch_fg = com_header.flag_header.exists_fg & 0b100;
		int fgment_id = com_header.mih.fgment_id;

		sketch[switch_id]->receive_fgment(sketch_fg, fgment_id, com_header.mih);
	}

};

}

#endif
