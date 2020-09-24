
/* -*- P4_16 -*- */
#include <core.p4>
#include <v1model.p4>

//My includes
#include "../include/headers.p4"
#include "../include/parsers.p4"

//test factors , volatile
#define BUCKET_NUM 64
#define BIN_NUM 8
#define BIN_CELL_BIT_WIDTH 32
#define RANDOM_BOUND 10


//data structure
#define ARRAY_REGISTER(num) register<bit<BIN_CELL_BIT_WIDTH>>(BUCKET_NUM * BIN_NUM) array##num
#define ARRAY_COUNTER(num) register<bit<1> >(BUCKET_NUM*3) counter##num

//6crc
//this is for sketch
#define COMPUTE_ARRAY_HASH(num) hash(meta.array_index##num,\
			 HashAlgorithm.crc32_custom,\
			 (bit<16>)0,\
			 {hdr.ipv4.srcAddr,\
			  hdr.ipv4.dstAddr,\
			  meta.ipv4_srcPort,\
			  meta.ipv4_dstPort,\
			  hdr.ipv4.protocol},\
			 (bit<32>)BUCKET_NUM);

//this is for timestamps
#define COMPUTE_TIMESTAMP_HASH(num)  hash(meta.timestamp_index##num,\
			HashAlgorithm.crc32_custom, \
			(bit<16>)0,\
			{hdr.ipv4.srcAddr,     \
			hdr.ipv4.dstAddr,     \
			meta.ipv4_srcPort,      \
			meta.ipv4_dstPort,      \
			hdr.ipv4.protocol},   \
			(bit<32>)BUCKET_NUM);       \

//1 crc32 custom
//this is for bring sketch fragments
/*
#define COMPUTE_SFH_HASH hash(meta.SFH_index0,\
			 HashAlgorithm.crc32,\
			 (bit<16>)0,\
			 {hdr.ipv4.srcAddr,\
			  hdr.ipv4.dstAddr,\
			  meta.ipv4_srcPort,\
			  meta.ipv4_dstPort,\
			  hdr.ipv4.protocol},\
			 (bit<32>)(BUCKET_NUM*3));
*/
//group0
ARRAY_REGISTER(0);
ARRAY_REGISTER(1);
ARRAY_REGISTER(2);

//group1
ARRAY_REGISTER(3);
ARRAY_REGISTER(4);
ARRAY_REGISTER(5);

//counter
ARRAY_COUNTER(0);
ARRAY_COUNTER(1);
ARRAY_COUNTER(2);


//control plane used
register<bit<8>>(1) sketch_fg;
register<bit<8>>(1) swap_control;

register<bit<16>>(1) switch_id;
register<bit<48>>(1) previous_ingress_timestamp;


/*************************************************************************
************   C H E C K S U M    V E R I F I C A T I O N   *************
*************************************************************************/

control MyVerifyChecksum(inout headers hdr, inout metadata meta)
{
	apply {}
}

/*************************************************************************
**************  I N G R E S S   P R O C E S S I N G   *******************
*************************************************************************/

control MyIngress(inout headers hdr,
				  inout metadata meta,
				  inout standard_metadata_t standard_metadata)
{

	//get the bringable bucket index 
	action predispose(){
		meta.SFH_index = 3 * BUCKET_NUM;

		random(meta.counter_index0, (bit<32>)0, (bit<32>)(3 * BUCKET_NUM - 1));
		counter0.read(meta.counter_value0,meta.counter_index0);
		if (meta.SFH_index >= 3 * BUCKET_NUM && meta.counter_value0 == 0) {
			meta.SFH_index = meta.counter_index0;
			meta.counter_value0 = 1;
		}
		counter0.write(meta.counter_index0, meta.counter_value0);
		
		random(meta.counter_index1, (bit<32>)0, (bit<32>)(3 * BUCKET_NUM - 1));
		counter1.read(meta.counter_value1, meta.counter_index1);
		if (meta.SFH_index >= 3 * BUCKET_NUM && meta.counter_value1 == 0) {
			meta.SFH_index = meta.counter_index1;
			meta.counter_value1 = 1;
		}
		counter1.write(meta.counter_index1, meta.counter_value1);
		
		random(meta.counter_index2, (bit<32>)0, (bit<32>)(3 * BUCKET_NUM - 1));
		counter2.read(meta.counter_value2, meta.counter_index2);
		if (meta.SFH_index >= 3 * BUCKET_NUM && meta.counter_value2 == 0) {
			meta.SFH_index = meta.counter_index2;
			meta.counter_value2 = 1;
		}
		counter2.write(meta.counter_index2, meta.counter_value2);

		random(meta.counter_index3, (bit<32>)0, (bit<32>)(3 * BUCKET_NUM - 1));
		if (meta.SFH_index >= 3 * BUCKET_NUM) {
			meta.SFH_index = meta.counter_index3;
		}
	}

	action _choose_fragment(bit<8> SFH_target_array){
		meta.SFH_target_bucket = meta.SFH_index - 
			(bit<32>)SFH_target_array * BUCKET_NUM;
		meta.SFH_target_array = SFH_target_array + (1 - meta.sketch_fg) * 3;
	}

	table choose_fragment{
		key={
			meta.SFH_index:range;
		}
		actions={
			NoAction;
			_choose_fragment;
		}
		size=256;
		default_action=NoAction;
	}

	action update_using_sketch0(){
		
	   array0.read(meta.tmp00,meta.SFH_target_bucket*BIN_NUM+0);
	   array0.read(meta.tmp01,meta.SFH_target_bucket*BIN_NUM+1);
	   array0.read(meta.tmp02,meta.SFH_target_bucket*BIN_NUM+2);
	   array0.read(meta.tmp03,meta.SFH_target_bucket*BIN_NUM+3);
	   array0.read(meta.tmp04,meta.SFH_target_bucket*BIN_NUM+4);
	   array0.read(meta.tmp05,meta.SFH_target_bucket*BIN_NUM+5);
	   array0.read(meta.tmp06,meta.SFH_target_bucket*BIN_NUM+6);
	   array0.read(meta.tmp07,meta.SFH_target_bucket*BIN_NUM+7);
	   //array0.read(meta.tmp08,meta.SFH_target_bucket*BIN_NUM+8);
	   //array0.read(meta.tmp09,meta.SFH_target_bucket*BIN_NUM+9);

	   array1.read(meta.tmp10,meta.SFH_target_bucket*BIN_NUM+0);
	   array1.read(meta.tmp11,meta.SFH_target_bucket*BIN_NUM+1);
	   array1.read(meta.tmp12,meta.SFH_target_bucket*BIN_NUM+2);
	   array1.read(meta.tmp13,meta.SFH_target_bucket*BIN_NUM+3);
	   array1.read(meta.tmp14,meta.SFH_target_bucket*BIN_NUM+4);
	   array1.read(meta.tmp15,meta.SFH_target_bucket*BIN_NUM+5);
	   array1.read(meta.tmp16,meta.SFH_target_bucket*BIN_NUM+6);
	   array1.read(meta.tmp17,meta.SFH_target_bucket*BIN_NUM+7);
	   //array1.read(meta.tmp18,meta.SFH_target_bucket*BIN_NUM+8);
	   //array1.read(meta.tmp19,meta.SFH_target_bucket*BIN_NUM+9);

	   array2.read(meta.tmp20,meta.SFH_target_bucket*BIN_NUM+0);
	   array2.read(meta.tmp21,meta.SFH_target_bucket*BIN_NUM+1);
	   array2.read(meta.tmp22,meta.SFH_target_bucket*BIN_NUM+2);
	   array2.read(meta.tmp23,meta.SFH_target_bucket*BIN_NUM+3);
	   array2.read(meta.tmp24,meta.SFH_target_bucket*BIN_NUM+4);
	   array2.read(meta.tmp25,meta.SFH_target_bucket*BIN_NUM+5);
	   array2.read(meta.tmp26,meta.SFH_target_bucket*BIN_NUM+6);
	   array2.read(meta.tmp27,meta.SFH_target_bucket*BIN_NUM+7);
	   //array2.read(meta.tmp28,meta.SFH_target_bucket*BIN_NUM+8);
	   //array2.read(meta.tmp29,meta.SFH_target_bucket*BIN_NUM+9);
		

	   if(meta.SFH_target_array==0){
		   hdr.SFH.sfh_delay0=meta.tmp00;
		   hdr.SFH.sfh_delay1=meta.tmp01;
		   hdr.SFH.sfh_delay2=meta.tmp02;
		   hdr.SFH.sfh_delay3=meta.tmp03;
		   hdr.SFH.sfh_delay4=meta.tmp04;
		   hdr.SFH.sfh_delay5=meta.tmp05;
		   hdr.SFH.sfh_delay6=meta.tmp06;
		   hdr.SFH.sfh_delay7=meta.tmp07;
		   //hdr.SFH.sfh_delay8=meta.tmp08;
		   //hdr.SFH.sfh_delay9=meta.tmp09;
	   }
	   else if(meta.SFH_target_array==1){
		   hdr.SFH.sfh_delay0=meta.tmp10;
		   hdr.SFH.sfh_delay1=meta.tmp11;
		   hdr.SFH.sfh_delay2=meta.tmp12;
		   hdr.SFH.sfh_delay3=meta.tmp13;
		   hdr.SFH.sfh_delay4=meta.tmp14;
		   hdr.SFH.sfh_delay5=meta.tmp15;
		   hdr.SFH.sfh_delay6=meta.tmp16;
		   hdr.SFH.sfh_delay7=meta.tmp17;
		   //hdr.SFH.sfh_delay8=meta.tmp18;
		   //hdr.SFH.sfh_delay9=meta.tmp19;
	   }
	   else{
		   hdr.SFH.sfh_delay0=meta.tmp20;
		   hdr.SFH.sfh_delay1=meta.tmp21;
		   hdr.SFH.sfh_delay2=meta.tmp22;
		   hdr.SFH.sfh_delay3=meta.tmp23;
		   hdr.SFH.sfh_delay4=meta.tmp24;
		   hdr.SFH.sfh_delay5=meta.tmp25;
		   hdr.SFH.sfh_delay6=meta.tmp26;
		   hdr.SFH.sfh_delay7=meta.tmp27;
		   //hdr.SFH.sfh_delay8=meta.tmp28;
		   //hdr.SFH.sfh_delay9=meta.tmp29;
	   }
	}

	action update_using_sketch1(){
	
	   array3.read(meta.tmp00,meta.SFH_target_bucket*BIN_NUM+0);
	   array3.read(meta.tmp01,meta.SFH_target_bucket*BIN_NUM+1);
	   array3.read(meta.tmp02,meta.SFH_target_bucket*BIN_NUM+2);
	   array3.read(meta.tmp03,meta.SFH_target_bucket*BIN_NUM+3);
	   array3.read(meta.tmp04,meta.SFH_target_bucket*BIN_NUM+4);
	   array3.read(meta.tmp05,meta.SFH_target_bucket*BIN_NUM+5);
	   array3.read(meta.tmp06,meta.SFH_target_bucket*BIN_NUM+6);
	   array3.read(meta.tmp07,meta.SFH_target_bucket*BIN_NUM+7);
	   //array3.read(meta.tmp08,meta.SFH_target_bucket*BIN_NUM+8);
	   //array3.read(meta.tmp09,meta.SFH_target_bucket*BIN_NUM+9);

	   array4.read(meta.tmp10,meta.SFH_target_bucket*BIN_NUM+0);
	   array4.read(meta.tmp11,meta.SFH_target_bucket*BIN_NUM+1);
	   array4.read(meta.tmp12,meta.SFH_target_bucket*BIN_NUM+2);
	   array4.read(meta.tmp13,meta.SFH_target_bucket*BIN_NUM+3);
	   array4.read(meta.tmp14,meta.SFH_target_bucket*BIN_NUM+4);
	   array4.read(meta.tmp15,meta.SFH_target_bucket*BIN_NUM+5);
	   array4.read(meta.tmp16,meta.SFH_target_bucket*BIN_NUM+6);
	   array4.read(meta.tmp17,meta.SFH_target_bucket*BIN_NUM+7);
	   //array4.read(meta.tmp18,meta.SFH_target_bucket*BIN_NUM+8);
	   //array4.read(meta.tmp19,meta.SFH_target_bucket*BIN_NUM+9);

	   array5.read(meta.tmp20,meta.SFH_target_bucket*BIN_NUM+0);
	   array5.read(meta.tmp21,meta.SFH_target_bucket*BIN_NUM+1);
	   array5.read(meta.tmp22,meta.SFH_target_bucket*BIN_NUM+2);
	   array5.read(meta.tmp23,meta.SFH_target_bucket*BIN_NUM+3);
	   array5.read(meta.tmp24,meta.SFH_target_bucket*BIN_NUM+4);
	   array5.read(meta.tmp25,meta.SFH_target_bucket*BIN_NUM+5);
	   array5.read(meta.tmp26,meta.SFH_target_bucket*BIN_NUM+6);
	   array5.read(meta.tmp27,meta.SFH_target_bucket*BIN_NUM+7);
	   //array5.read(meta.tmp28,meta.SFH_target_bucket*BIN_NUM+8);
	   //array5.read(meta.tmp29,meta.SFH_target_bucket*BIN_NUM+9);

	   if(meta.SFH_target_array==3){
		   hdr.SFH.sfh_delay0=meta.tmp00;
		   hdr.SFH.sfh_delay1=meta.tmp01;
		   hdr.SFH.sfh_delay2=meta.tmp02;
		   hdr.SFH.sfh_delay3=meta.tmp03;
		   hdr.SFH.sfh_delay4=meta.tmp04;
		   hdr.SFH.sfh_delay5=meta.tmp05;
		   hdr.SFH.sfh_delay6=meta.tmp06;
		   hdr.SFH.sfh_delay7=meta.tmp07;
		   //hdr.SFH.sfh_delay8=meta.tmp08;
		   //hdr.SFH.sfh_delay9=meta.tmp09;
	   }
	   else if(meta.SFH_target_array==4){
		   hdr.SFH.sfh_delay0=meta.tmp10;
		   hdr.SFH.sfh_delay1=meta.tmp11;
		   hdr.SFH.sfh_delay2=meta.tmp12;
		   hdr.SFH.sfh_delay3=meta.tmp13;
		   hdr.SFH.sfh_delay4=meta.tmp14;
		   hdr.SFH.sfh_delay5=meta.tmp15;
		   hdr.SFH.sfh_delay6=meta.tmp16;
		   hdr.SFH.sfh_delay7=meta.tmp17;
		   //hdr.SFH.sfh_delay8=meta.tmp18;
		   //hdr.SFH.sfh_delay9=meta.tmp19;
	   }
	   else{
		   hdr.SFH.sfh_delay0=meta.tmp20;
		   hdr.SFH.sfh_delay1=meta.tmp21;
		   hdr.SFH.sfh_delay2=meta.tmp22;
		   hdr.SFH.sfh_delay3=meta.tmp23;
		   hdr.SFH.sfh_delay4=meta.tmp24;
		   hdr.SFH.sfh_delay5=meta.tmp25;
		   hdr.SFH.sfh_delay6=meta.tmp26;
		   hdr.SFH.sfh_delay7=meta.tmp27;
		   //hdr.SFH.sfh_delay8=meta.tmp28;
		   //hdr.SFH.sfh_delay9=meta.tmp29;
	   }
	}

	table update_SFH{
		key={
			meta.sketch_fg:exact;
		}
		actions={
			NoAction;
			update_using_sketch0;
			update_using_sketch1;
		}
		size=256;
		default_action=NoAction;
	}

	/******************* inherited code starts here       ************************/
	action drop(){
		mark_to_drop();
	}

	action ecmp_group(bit<14> ecmp_group_id, bit<16> num_nhops)
	{
		bit<32> tmp;
		random(tmp, (bit<32>)0, (bit<32>)RANDOM_BOUND - 1);
		hash(meta.ecmp_hash,
			HashAlgorithm.crc16,
			(bit<1>)0,
			{hdr.ipv4.srcAddr,
			hdr.ipv4.dstAddr,
			meta.ipv4_srcPort,
			meta.ipv4_dstPort,
			hdr.ipv4.protocol,
			 tmp
			},
			num_nhops);

		meta.ecmp_group_id = ecmp_group_id;
	}

	action set_nhop(macAddr_t dstAddr, egressSpec_t port)
	{
		//set the src mac address as the previous dst, this is not correct right?
		hdr.ethernet.srcAddr = hdr.ethernet.dstAddr;
		//set the destination mac address that we got from the match in the table
		hdr.ethernet.dstAddr = dstAddr;
		//set the output port that we also get from the table
		standard_metadata.egress_spec = port;
		//decrease ttl by 1
		hdr.ipv4.ttl = hdr.ipv4.ttl - 1;
	}

	table ecmp_group_to_nhop
	{
		key = {
			meta.ecmp_group_id : exact;
			meta.ecmp_hash : exact;
		}
		actions = {
			drop;
			set_nhop;
		}
		size = 1024;
	}

	table ipv4_lpm
	{
		key = {
			hdr.ipv4.dstAddr : lpm;
		}
		actions = {
		   set_nhop;
		ecmp_group;
		drop;
		}
		size = 1024;
		default_action = drop;
	}

	/******************* inherited code ends here       ************************/



	/******** log code starts here*******/

	action send_to_control_plane(){
		meta.switch_delay = standard_metadata.egress_global_timestamp-standard_metadata.ingress_global_timestamp;
		clone3(CloneType.E2E,100,meta);//mirror id = 100
	}

	/******** log code ends here*******/

	apply
	{   
		if (hdr.ipv4.isValid()&&hdr.ipv4.ttl > 1) {
			

			/******** log code starts here*******/
			//send_to_control_plane();
			/******** log code ends here*******/
			
			if(hdr.udp.isValid()||hdr.tcp.isValid()){
				switch_id.read(meta.switch_id, 0);
				sketch_fg.read(meta.sketch_fg,0);
				swap_control.read(meta.swap_control,0);//0 bring-able ;1 forbidden

				if (meta.swap_control == 0 && (!hdr.SFH.isValid())) {//this packet is vacant 
					random(meta.random_number, (bit<32>)0, (bit<32>)RANDOM_BOUND - 1);
					if (meta.random_number <= 1) {//the probility allows
						meta.SFH_index = 3 * BUCKET_NUM;

						predispose();//compute the index of bucket

						if (meta.SFH_index < 3 * BUCKET_NUM) {
							if(hdr.udp.isValid()){
								hdr.udp.checksum = 0;
								hdr.ipv4.totalLen = hdr.ipv4.totalLen + (38);
								hdr.udp.length = hdr.udp.length + (38);
								hdr.flag.flag=hdr.flag.flag| 0b010;
								hdr.flag.flag= hdr.flag.flag & 0b1111_1110;
								hdr.flag.flag =hdr.flag.flag |(1 - meta.sketch_fg);
							
							}
							else if(hdr.tcp.isValid()) {
								hdr.ipv4.totalLen = hdr.ipv4.totalLen + (38);
								hdr.tcp.SFH_fg = 1;
								hdr.tcp.SFH_sketch_number = (bit<1>)(1 - meta.sketch_fg);
								//abandoned
							}

							hdr.SFH.setValid();
							hdr.SFH.sfh_switch_id = meta.switch_id;
							hdr.SFH.sfh_fgment_id = meta.SFH_index;
							
							choose_fragment.apply();
							update_SFH.apply();
						}
					}
					random(meta.random_number, (bit<32>)0, (bit<32>)10);
					if(meta.random_number==5){
						standard_metadata.priority=7;
					}
					else{
						standard_metadata.priority=0;
					}
				}
			}
			switch (ipv4_lpm.apply().action_run){
				ecmp_group:{
					ecmp_group_to_nhop.apply();
				}
			}
		}
		else{
			drop();
		}
		
	}
	
}

/*************************************************************************
****************  E G R E S S   P R O C E S S I N G   *******************
*************************************************************************/

control MyEgress(inout headers hdr,
				 inout metadata meta,
				 inout standard_metadata_t standard_metadata)
{
	//function:we can only get the egress time in this part ,so we implement the insert of delay here


	//this action get the corresponding lev of bucket
	action _get_delay_lev(bit<32> delay_lev){
		meta.delay_lev = delay_lev;
	}

	//semi-CU-sketch    
	//update sketch 0 : array0\1\2
	action update_sketch0(){
		COMPUTE_ARRAY_HASH(0)
		COMPUTE_ARRAY_HASH(1)
		COMPUTE_ARRAY_HASH(2)

		bit<32>tmp_min=0x5fffffff;

		array0.read(meta.array_value0, (meta.array_index0 * BIN_NUM + meta.delay_lev));
		if(tmp_min>=meta.array_value0){
			tmp_min=meta.array_value0;
			meta.array_value0 = meta.array_value0 + 1;
		}

		array1.read(meta.array_value1, (meta.array_index1 * BIN_NUM + meta.delay_lev));
		if(tmp_min>=meta.array_value1){
			tmp_min=meta.array_value1;
			meta.array_value1 = meta.array_value1 + 1;
		}

		array2.read(meta.array_value2, (meta.array_index2 * BIN_NUM + meta.delay_lev));
		if(tmp_min>=meta.array_value2){
			tmp_min=meta.array_value2;
			meta.array_value2 = meta.array_value2 + 1;
		}

		array0.write((meta.array_index0 * BIN_NUM + meta.delay_lev), meta.array_value0);
		array1.write((meta.array_index1 * BIN_NUM + meta.delay_lev), meta.array_value1);
		array2.write((meta.array_index2 * BIN_NUM + meta.delay_lev), meta.array_value2);
		
	}

	//update sketch 1 : array3\4\5
	action update_sketch1(){
		COMPUTE_ARRAY_HASH(3)
		COMPUTE_ARRAY_HASH(4)
		COMPUTE_ARRAY_HASH(5)

		bit<32>tmp_min=0x5fff_ffff;

		array3.read(meta.array_value3, (meta.array_index3 * BIN_NUM + meta.delay_lev));
		if(tmp_min>=meta.array_value3){
			tmp_min=meta.array_value3;
			meta.array_value3 = meta.array_value3 + 1;
		}

		array4.read(meta.array_value4, (meta.array_index4 * BIN_NUM + meta.delay_lev));
		if(tmp_min>=meta.array_value4){
			tmp_min=meta.array_value4;
			meta.array_value4 = meta.array_value4 + 1;
		}

		array5.read(meta.array_value5, (meta.array_index5 * BIN_NUM + meta.delay_lev));
		if(tmp_min>=meta.array_value5){
			tmp_min=meta.array_value5;
			meta.array_value5 = meta.array_value5 + 1;
		}

		array3.write((meta.array_index3 * BIN_NUM + meta.delay_lev), meta.array_value3);
		array4.write((meta.array_index4 * BIN_NUM + meta.delay_lev), meta.array_value4);
		array5.write((meta.array_index5 * BIN_NUM + meta.delay_lev), meta.array_value5);
	}


	table update_sketch{
		key = {
			meta.sketch_fg : exact;
		}
		actions = {
			update_sketch0;
			update_sketch1;
			NoAction;
		}
		size = 256;
		default_action = NoAction;
	}

	table get_delay_lev{
		key={
			meta.switch_delay:range;
		}
		actions={
			_get_delay_lev;
			NoAction;
		}
		size=256;
		default_action=NoAction;
	}

	
	/******** log code starts here*******/

	action send_to_control_plane(){
		//meta.switch_delay = standard_metadata.egress_global_timestamp-standard_metadata.ingress_global_timestamp;
		clone3(CloneType.E2E,100,meta);//mirror id = 100
	}

	/******** log code ends here*******/

	apply
	{
		if(hdr.ipv4.isValid()&&standard_metadata.instance_type ==0&&hdr.ipv4.ttl > 1){

			meta.switch_delay = standard_metadata.egress_global_timestamp-standard_metadata.ingress_global_timestamp;
			
			previous_ingress_timestamp.read(meta.previous_ingress_global_timestamp,(bit<32>)0);
			meta.interval=standard_metadata.ingress_global_timestamp-meta.previous_ingress_global_timestamp;
			previous_ingress_timestamp.write((bit<32>)0,standard_metadata.ingress_global_timestamp);
			
			//sketch_fgsketch_fg.read(meta.sketch_fg,0);  this has been read in ingress
			
			//temporarily store whether the sfh exists
			if(hdr.tcp.isValid()){
				meta.swap_control = (bit<8>)hdr.tcp.SFH_fg;
			}
			else if(hdr.udp.isValid()){
				meta.swap_control = (hdr.flag.flag&0b010)>>1;
			}
			
			/*********  log code starts here  **********/
			send_to_control_plane();
			/********  log code ends here ***********/

			get_delay_lev.apply();
			update_sketch.apply();
		}

		/************************ log code starts here**********************/
		if (standard_metadata.instance_type == 2){
			//ether: 16   Ipv4:20  tcp:20  udp:8  mih:11  sfh:47
			hdr.CPU.setValid();
			
			hdr.CPU.srcAddr=hdr.ipv4.srcAddr;
			hdr.CPU.dstAddr=hdr.ipv4.dstAddr;
			hdr.CPU.protocol=hdr.ipv4.protocol;
			hdr.CPU.srcPort=meta.ipv4_srcPort;
			hdr.CPU.dstPort=meta.ipv4_dstPort;
			hdr.CPU.delay=meta.switch_delay;
			hdr.CPU.interval=meta.interval;
			hdr.CPU.flags=(meta.sketch_fg<<1) |(meta.swap_control);
			hdr.CPU.flags=hdr.CPU.flags&0b1111_1011;


			hdr.ethernet.setInvalid();
			hdr.ipv4.setInvalid();
			hdr.tcp.setInvalid();
			hdr.udp.setInvalid();
			hdr.flag.setInvalid();
			hdr.MIH.setInvalid();
			hdr.SFH.setInvalid();
		}
		/************************ log code ends here***************************/
		
	}
}

/*************************************************************************
*************   C H E C K S U M    C O M P U T A T I O N   **************
*************************************************************************/

control MyComputeChecksum(inout headers hdr, inout metadata meta)
{
	apply
	{
		update_checksum(
			hdr.ipv4.isValid(),
			{hdr.ipv4.version,
			 hdr.ipv4.ihl,
			 hdr.ipv4.dscp,
			 hdr.ipv4.ecn,
			 hdr.ipv4.totalLen,
			 hdr.ipv4.identification,
			 hdr.ipv4.flags,
			 hdr.ipv4.fragOffset,
			 hdr.ipv4.ttl,
			 hdr.ipv4.protocol,
			 hdr.ipv4.srcAddr,
			 hdr.ipv4.dstAddr},
			hdr.ipv4.hdrChecksum,
			HashAlgorithm.csum16);

		

		update_checksum_with_payload(
			hdr.SFH.isValid(),
			{hdr.ipv4.srcAddr,
			 hdr.ipv4.dstAddr,
			 8w0,
			 hdr.ipv4.protocol,
			 hdr.udp.length,
			 hdr.udp.srcPort,
			 hdr.udp.dstPort,
			 hdr.udp.length,
			 hdr.flag,
			 hdr.SFH},
			hdr.udp.checksum,
			HashAlgorithm.csum16);
	}
}

/*************************************************************************
***********************  S W I T C H  *******************************
*************************************************************************/

//switch architecture
V1Switch(
	MyParser(),
	MyVerifyChecksum(),
	MyIngress(),
	MyEgress(),
	MyComputeChecksum(),
	MyDeparser()) main;
