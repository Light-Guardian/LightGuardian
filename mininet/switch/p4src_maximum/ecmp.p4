
/* -*- P4_16 -*- */
#include <core.p4>
#include <v1model.p4>

//My includes
#include "../include/headers.p4"
#include "../include/parsers.p4"

//test factors , volatile
#define BUCKET_NUM 64
#define BIN_NUM 10
#define BIN_CELL_BIT_WIDTH 32
#define RANDOM_BOUND 10

#define ARRAY_COUNTER(num) register<bit<1> >(BUCKET_NUM*3) counter##num
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


//timestamps for the maximum
register<bit<48>>(BUCKET_NUM) max_interval_array0;
register<bit<48>>(BUCKET_NUM) max_interval_array1;
register<bit<48>>(BUCKET_NUM) max_interval_array2;

register<bit<48>>(BUCKET_NUM) max_interval_array3;
register<bit<48>>(BUCKET_NUM) max_interval_array4;
register<bit<48>>(BUCKET_NUM) max_interval_array5;

//timestamps of the last incoming packet
register<bit<48>>(BUCKET_NUM) timestamp_array0;
register<bit<48>>(BUCKET_NUM) timestamp_array1;
register<bit<48>>(BUCKET_NUM) timestamp_array2;

//counter
ARRAY_COUNTER(0);
ARRAY_COUNTER(1);
ARRAY_COUNTER(2);

//control plane 
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
    action update_interval_using_sketch0(){
        COMPUTE_TIMESTAMP_HASH(0)
        COMPUTE_TIMESTAMP_HASH(1)
        COMPUTE_TIMESTAMP_HASH(2)
        
        timestamp_array0.read(meta.timestamp_value0,meta.timestamp_index0);
        timestamp_array1.read(meta.timestamp_value1,meta.timestamp_index1);
        timestamp_array2.read(meta.timestamp_value2,meta.timestamp_index2);

        max_interval_array0.read(meta.max_interval_value0,meta.timestamp_index0);
        max_interval_array1.read(meta.max_interval_value1,meta.timestamp_index1);
        max_interval_array2.read(meta.max_interval_value2,meta.timestamp_index2);

        if(meta.timestamp_value0>meta.timestamp_value1&&meta.timestamp_value1!=0){
            meta.timestamp_value0=meta.timestamp_value1;
        }
        if(meta.timestamp_value0>meta.timestamp_value2&&meta.timestamp_value2!=0){
            meta.timestamp_value0=meta.timestamp_value2;
        }
        if(meta.timestamp_value0!=0){
            meta.timestamp_value0=standard_metadata.ingress_global_timestamp-meta.timestamp_value0;

            if(meta.max_interval_value0<meta.timestamp_value0){
                meta.max_interval_value0=meta.timestamp_value0;
            }
            if(meta.max_interval_value1<meta.timestamp_value0){
                meta.max_interval_value1=meta.timestamp_value0;
            }
            if(meta.max_interval_value2<meta.timestamp_value0){
                meta.max_interval_value2=meta.timestamp_value0;
            }
        }
        max_interval_array0.write(meta.timestamp_index0,meta.max_interval_value0);
        max_interval_array1.write(meta.timestamp_index1,meta.max_interval_value1);
        max_interval_array2.write(meta.timestamp_index2,meta.max_interval_value2);

        timestamp_array0.write(meta.timestamp_index0,standard_metadata.ingress_global_timestamp);
        timestamp_array1.write(meta.timestamp_index1,standard_metadata.ingress_global_timestamp);
        timestamp_array2.write(meta.timestamp_index2,standard_metadata.ingress_global_timestamp);
    }

    action update_interval_using_sketch1(){
        COMPUTE_TIMESTAMP_HASH(0)
        COMPUTE_TIMESTAMP_HASH(1)
        COMPUTE_TIMESTAMP_HASH(2)
        
        timestamp_array0.read(meta.timestamp_value0,meta.timestamp_index0);
        timestamp_array1.read(meta.timestamp_value1,meta.timestamp_index1);
        timestamp_array2.read(meta.timestamp_value2,meta.timestamp_index2);

        max_interval_array3.read(meta.max_interval_value0,meta.timestamp_index0);
        max_interval_array4.read(meta.max_interval_value1,meta.timestamp_index1);
        max_interval_array5.read(meta.max_interval_value2,meta.timestamp_index2);

        if(meta.timestamp_value0>meta.timestamp_value1&&meta.timestamp_value1!=0){
            meta.timestamp_value0=meta.timestamp_value1;
        }
        if(meta.timestamp_value0>meta.timestamp_value2&&meta.timestamp_value2!=0){
            meta.timestamp_value0=meta.timestamp_value2;
        }
        if(meta.timestamp_value0!=0){
            meta.timestamp_value0=standard_metadata.ingress_global_timestamp-meta.timestamp_value0;

            if(meta.max_interval_value0<meta.timestamp_value0){
                meta.max_interval_value0=meta.timestamp_value0;
            }
            if(meta.max_interval_value1<meta.timestamp_value0){
                meta.max_interval_value1=meta.timestamp_value0;
            }
            if(meta.max_interval_value2<meta.timestamp_value0){
                meta.max_interval_value2=meta.timestamp_value0;
            }
        }
        max_interval_array3.write(meta.timestamp_index0,meta.max_interval_value0);
        max_interval_array4.write(meta.timestamp_index1,meta.max_interval_value1);
        max_interval_array5.write(meta.timestamp_index2,meta.max_interval_value2);

        timestamp_array0.write(meta.timestamp_index0,standard_metadata.ingress_global_timestamp);
        timestamp_array1.write(meta.timestamp_index1,standard_metadata.ingress_global_timestamp);
        timestamp_array2.write(meta.timestamp_index2,standard_metadata.ingress_global_timestamp);
    }

    table update_interval{
        key={
            meta.sketch_fg:exact;
        }
        actions={
            NoAction;
            update_interval_using_sketch0;
            update_interval_using_sketch1;
        }
        size=256;
        default_action=NoAction;
    }


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
            switch_id.read(meta.switch_id, 0);
            sketch_fg.read(meta.sketch_fg,0);
            swap_control.read(meta.swap_control,0);//0 bring-able ;1 forbidden
            
            update_interval.apply();
            
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
    //get the bringable bucket index 
	action predispose(){
		meta.MIH_index = 3 * BUCKET_NUM;

		random(meta.counter_index0, (bit<32>)0, (bit<32>)(3 * BUCKET_NUM - 1));
		counter0.read(meta.counter_value0,meta.counter_index0);
		if (meta.MIH_index >= 3 * BUCKET_NUM && meta.counter_value0 == 0) {
			meta.MIH_index = meta.counter_index0;
			meta.counter_value0 = 1;
		}
		counter0.write(meta.counter_index0, meta.counter_value0);
		
		random(meta.counter_index1, (bit<32>)0, (bit<32>)(3 * BUCKET_NUM - 1));
		counter1.read(meta.counter_value1, meta.counter_index1);
		if (meta.MIH_index >= 3 * BUCKET_NUM && meta.counter_value1 == 0) {
			meta.MIH_index = meta.counter_index1;
			meta.counter_value1 = 1;
		}
		counter1.write(meta.counter_index1, meta.counter_value1);
		
		random(meta.counter_index2, (bit<32>)0, (bit<32>)(3 * BUCKET_NUM - 1));
		counter2.read(meta.counter_value2, meta.counter_index2);
		if (meta.MIH_index >= 3 * BUCKET_NUM && meta.counter_value2 == 0) {
			meta.MIH_index = meta.counter_index2;
			meta.counter_value2 = 1;
		}
		counter2.write(meta.counter_index2, meta.counter_value2);

		random(meta.counter_index3, (bit<32>)0, (bit<32>)(3 * BUCKET_NUM - 1));
		if (meta.MIH_index >= 3 * BUCKET_NUM) {
			meta.MIH_index = meta.counter_index3;
		}
	}

    action _choose_fragment(bit<8> MIH_target_array){
		meta.MIH_target_bucket = meta.MIH_index - 
			(bit<32>)MIH_target_array * BUCKET_NUM;
		meta.MIH_target_array = MIH_target_array + (1 - meta.sketch_fg) * 3;
	}

    table choose_fragment{
		key={
			meta.MIH_index:range;
		}
		actions={
			NoAction;
			_choose_fragment;
		}
		size=256;
		default_action=NoAction;
	}
    //update timestamp and ax interval

    action update_MIH_using_sketch0()
    {
        // from sketch
        max_interval_array0.read(meta.max_interval_value0,meta.MIH_target_bucket);
        max_interval_array1.read(meta.max_interval_value1,meta.MIH_target_bucket);
        max_interval_array2.read(meta.max_interval_value2,meta.MIH_target_bucket);
        
        if(meta.MIH_target_array==0){
            hdr.MIH.mih_timestamp=meta.max_interval_value0;
        }
        else if(meta.MIH_target_array==1){
            hdr.MIH.mih_timestamp=meta.max_interval_value1;
        }
        else if(meta.MIH_target_array==2){
            hdr.MIH.mih_timestamp=meta.max_interval_value2;
        }
    }

    action update_MIH_using_sketch1(){
        max_interval_array3.read(meta.max_interval_value0,meta.MIH_target_bucket);
        max_interval_array4.read(meta.max_interval_value1,meta.MIH_target_bucket);
        max_interval_array5.read(meta.max_interval_value2,meta.MIH_target_bucket);
        
        if(meta.MIH_target_array==3){
            hdr.MIH.mih_timestamp=meta.max_interval_value0;
        }
        else if(meta.MIH_target_array==4){
            hdr.MIH.mih_timestamp=meta.max_interval_value1;
        }
        else if(meta.MIH_target_array==5){
            hdr.MIH.mih_timestamp=meta.max_interval_value2;
        }
    }

    table update_MIH{
        key={
            meta.sketch_fg:exact;
        }
        actions={
            update_MIH_using_sketch0;
            update_MIH_using_sketch1;
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

            if(hdr.udp.isValid()||hdr.tcp.isValid()){
                if(meta.swap_control==0&&!hdr.MIH.isValid()){
                    random(meta.random_number, (bit<32>)0, (bit<32>)RANDOM_BOUND - 1);
                    if (meta.random_number <= 1) {
                        meta.MIH_index = 3 * BUCKET_NUM;

						predispose();
                        
                        if(meta.MIH_index < 3 * BUCKET_NUM){
                            if(hdr.udp.isValid()){
                                hdr.udp.checksum = 0;
                                hdr.ipv4.totalLen = hdr.ipv4.totalLen + (14);
                                hdr.udp.length = hdr.udp.length + (14);
                                hdr.flag.flag=hdr.flag.flag| 0b1000;
                                hdr.flag.flag=hdr.flag.flag& 0b1111_1011;
                                hdr.flag.flag=hdr.flag.flag|((1 - meta.sketch_fg)<<3);
                            
                            }
                            else if(hdr.tcp.isValid()) {
                                hdr.ipv4.totalLen = hdr.ipv4.totalLen + (14);
                                hdr.tcp.MIH_fg=1;
                                //abandoned
                            }
                            hdr.MIH.setValid();
                            hdr.MIH.mih_switch_id = meta.switch_id;
							hdr.MIH.mih_fgment_id = meta.MIH_index;
                            choose_fragment.apply();
                            update_MIH.apply();
                        }
                    }
                }
            }   
            meta.switch_delay = standard_metadata.egress_global_timestamp-standard_metadata.ingress_global_timestamp;
            
            previous_ingress_timestamp.read(meta.previous_ingress_global_timestamp,(bit<32>)0);
            meta.interval=standard_metadata.ingress_global_timestamp-meta.previous_ingress_global_timestamp;
            previous_ingress_timestamp.write((bit<32>)0,standard_metadata.ingress_global_timestamp);
            
            //sketch_fg.read(meta.sketch_fg,0);
            
            //temporarily store whether the sfh exists
            if(hdr.tcp.isValid()){
                meta.swap_control = (bit<8>)hdr.tcp.SFH_fg;
            }
            else  if(hdr.udp.isValid()){
                meta.swap_control = (hdr.flag.flag&0b1000)>>3;
            }
            
            /*********  log code starts here  ********* */
            send_to_control_plane();
            /********  log code ends here ********** */

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
            hdr.CPU.flags=hdr.CPU.flags|0b0000_0100;

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
			hdr.MIH.isValid(),
			{hdr.ipv4.srcAddr,
			 hdr.ipv4.dstAddr,
			 8w0,
			 hdr.ipv4.protocol,
			 hdr.udp.length,
			 hdr.udp.srcPort,
			 hdr.udp.dstPort,
			 hdr.udp.length,
			 hdr.flag,
             hdr.MIH
			 },
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
