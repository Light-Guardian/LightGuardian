/* -*- P4_14 -*- */
#include <tofino/constants.p4>
#include <tofino/intrinsic_metadata.p4>
#include <tofino/primitives.p4>
//Include the blackbox definition
#include <tofino/stateful_alu_blackbox.p4>
#include "includes/header_and_parser.p4"
#define CM_LEN 65536
#define FS_LEN 65536
#define id 7
header_type user_metadata_t {
    fields {
		hashvalue:1;
		tstamp:32;
		dts1:32;
		portnum:32;
		dts2:32;
		dts3:32;
		maxd:32;
		deltat:18;
		ost:4;
		loc1:16;
		loc2:16;
		loc3:16;
		loc4:16;
		loc5:16;
		loc6:16;
		loc7:16;
		loc8:16;
		ord1:4;
		ord2:4;
		flag:4;
		sign:2;
		goodsign1:32;
		goodsign2:32;
		goodsign3:32;
		goodsign4:32;
		cl:16;
		tag:4;
		fdt:32;
		fmin:32;
		fa_1_1:32;
		fa_2_1:32;
		fb_1_1:32;
		fb_2_1:32;
		fc_1_1:32;
		fc_2_1:32;
		fd_1_1:32;
		fd_2_1:32;
		cks:16;
		pad:8;

		dst:32;
		ram1:1;
		ram2:1;
		ramloc:16;
    }
}


metadata user_metadata_t m;

field_list udp_list{
    ipv4.proto;
    ipv4.sip;
    ipv4.dip;
    udp.sPort;
    udp.dPort;
}
field_list tcp_list{
    ipv4.proto;
    ipv4.sip;
    ipv4.dip;
    tcp.sPort;
    tcp.dPort;
}
field_list ipv4_list{
    ipv4.sip;
    ipv4.dip;
}
field_list arp_list{
    arp.pSrc;
    arp.pDst;
}

field_list_calculation hash_tcp {
    input {tcp_list;}
    algorithm: crc32;
    output_width: 1;
}
field_list_calculation hash_udp {
    input {udp_list;}
    algorithm: crc32;
    output_width: 1;
}
field_list_calculation hash_arp{
    input {arp_list;}
    algorithm: crc32;
    output_width: 1;
}
field_list_calculation hash_ipv4{
    input {ipv4_list;}
    algorithm: crc32;
    output_width: 1;
}


field_list flow_id_list1{
    ipv4.proto;
    ipv4.sip;
    ipv4.dip;
    tcp.sPort;
    tcp.dPort;
}

field_list flow_id_list4{
	m.tstamp;
}
field_list_calculation S_1 {
    input {flow_id_list1;}
    algorithm: crc8;
    output_width: 16;
}
field_list_calculation S_2 {
    input {flow_id_list1;}
    algorithm: crc16;
    output_width: 16;
}
field_list_calculation S_3 {
    input {flow_id_list1;}
    algorithm: crc32;
    output_width: 16;
}
field_list_calculation S_4 {
    input {flow_id_list1;}
    algorithm: random;
    output_width: 16;
}
register rm_a {
    width : 1;
    instance_count: FS_LEN;
}



register ts_1 {
    width : 64;
    instance_count: CM_LEN;
}
register ts_2 {
    width : 64;
    instance_count: CM_LEN;
}
register ts_3 {
    width : 64;
    instance_count: CM_LEN;
}

blackbox stateful_alu calts_3
{
	reg:ts_3;
	update_lo_1_value:m.tstamp;
	update_hi_1_value:m.tstamp-register_lo;
	output_value:alu_hi;
	output_dst:m.dts3;
}

action cs_3() {
    calts_3.execute_stateful_alu_from_hash(S_3);
}

table c_3
{
    actions { cs_3; }
    default_action :cs_3;
}




blackbox stateful_alu calts_1
{
	reg:ts_1;
	update_lo_1_value:m.tstamp;
	update_hi_1_value:m.tstamp-register_lo;
	output_value:alu_hi;
	output_dst:m.dts1;
}
blackbox stateful_alu calts_2
{
	reg:ts_2;
	update_lo_1_value:m.tstamp;
	update_hi_1_value:m.tstamp-register_lo;
	output_value:alu_hi;
	output_dst:m.dts2;
}


action cs_1() {
    calts_1.execute_stateful_alu_from_hash(S_1);
}
action cs_2() {
    calts_2.execute_stateful_alu_from_hash(S_2);
}


table c_1
{
    actions { cs_1; }
    default_action :cs_1;
}
table c_2
{
    actions { cs_2; }
    default_action :cs_2;
}




//fsketch
register fs_a_1_1 {
    width : 32;
    instance_count: FS_LEN;
}

register fs_a_2_1 {
    width : 32;
    instance_count: FS_LEN;
}


register fs_b_1_1 {
    width : 32;
    instance_count: FS_LEN;
}


register fs_b_2_1 {
    width : 32;
    instance_count: FS_LEN;
}

register fs_c_1_1 {
    width : 32;
    instance_count: FS_LEN;
}


register fs_c_2_1 {
    width : 32;
    instance_count: FS_LEN;
}

register fs_d_1_1 {
    width : 32;
    instance_count: FS_LEN;
}


register fs_d_2_1 {
    width : 32;
    instance_count: FS_LEN;
}



blackbox stateful_alu ffs_a_1_1
{
	reg:fs_a_1_1;
	condition_lo:m.goodsign1==1;
	condition_hi:m.fdt>register_lo;
	update_lo_1_predicate: condition_lo and condition_hi;
	update_lo_1_value:m.fdt;
	output_value:register_lo;
	output_dst:m.fa_1_1;
}





blackbox stateful_alu ffs_a_2_1
{
	reg:fs_a_2_1;
	condition_lo:m.goodsign2==5;
	condition_hi:m.fdt>register_lo;
	update_lo_1_predicate: condition_lo and condition_hi;
	update_lo_1_value:m.fdt;
	output_value:register_lo;
	output_dst:m.fa_2_1;
}







blackbox stateful_alu ffs_b_1_1
{
	reg:fs_b_1_1;
	condition_lo:m.goodsign3==1;
	condition_hi:m.fdt>register_lo;
	update_lo_1_predicate: condition_lo and condition_hi;
	update_lo_1_value:m.fdt;
	output_value:register_lo;
	output_dst:m.fb_1_1;
}



blackbox stateful_alu ffs_b_2_1
{
	reg:fs_b_2_1;
condition_lo:m.goodsign4==5;
	condition_hi:m.fdt>register_lo;
	update_lo_1_predicate: condition_lo and condition_hi;
	update_lo_1_value:m.fdt;
	output_value:register_lo;
	output_dst:m.fb_2_1;
}

blackbox stateful_alu ffs_c_1_1
{
	reg:fs_c_1_1;
	condition_lo:m.goodsign3==1;
	condition_hi:m.fdt>register_lo;
	update_lo_1_predicate: condition_lo and condition_hi;
	update_lo_1_value:m.fdt;
	output_value:register_lo;
	output_dst:m.fc_1_1;
}
blackbox stateful_alu ffs_c_2_1
{
	reg:fs_c_2_1;
condition_lo:m.goodsign4==5;
	condition_hi:m.fdt>register_lo;
	update_lo_1_predicate: condition_lo and condition_hi;
	update_lo_1_value:m.fdt;
	output_value:register_lo;
	output_dst:m.fc_2_1;
}
blackbox stateful_alu ffs_d_1_1
{
	reg:fs_d_1_1;
	condition_lo:m.goodsign3==1;
	condition_hi:m.fdt>register_lo;
	update_lo_1_predicate: condition_lo and condition_hi;
	update_lo_1_value:m.fdt;
	output_value:register_lo;
	output_dst:m.fd_1_1;
}



blackbox stateful_alu ffs_d_2_1
{
	reg:fs_d_2_1;
condition_lo:m.goodsign4==5;
	condition_hi:m.fdt>register_lo;
	update_lo_1_predicate: condition_lo and condition_hi;
	update_lo_1_value:m.fdt;
	output_value:register_lo;
	output_dst:m.fd_2_1;
}



action fffs_a_1_1() {
    ffs_a_1_1.execute_stateful_alu(m.loc1);
}


action fffs_a_2_1() {
    ffs_a_2_1.execute_stateful_alu(m.loc5);
}

action fffs_b_1_1() {
    ffs_b_1_1.execute_stateful_alu(m.loc2);
}



action fffs_b_2_1() {
    ffs_b_2_1.execute_stateful_alu(m.loc6);
}

action fffs_c_1_1() {
    ffs_c_1_1.execute_stateful_alu(m.loc3);
}


action fffs_c_2_1() {
    ffs_c_2_1.execute_stateful_alu(m.loc7);
}

action fffs_d_1_1() {
    ffs_d_1_1.execute_stateful_alu(m.loc4);
}

action fffs_d_2_1() {
    ffs_d_2_1.execute_stateful_alu(m.loc8);
}


table insert_a_1_1 {
   
	actions 
	{
        
		fffs_a_1_1;
		
    }
	default_action:fffs_a_1_1;
}



table insert_a_2_1 {
   
	actions 
	{
        
		fffs_a_2_1;
		
    }
	default_action:fffs_a_2_1;
}




table insert_b_1_1 {
   
	actions 
	{
        
		fffs_b_1_1;
		
    }
	default_action:fffs_b_1_1;
}


table insert_b_2_1 {
   
	actions 
	{
        
		fffs_b_2_1;
		
    }
	default_action:fffs_b_2_1;
}

table insert_c_1_1 {
   
	actions 
	{
        
		fffs_c_1_1;
		
    }
	default_action:fffs_c_1_1;
}



table insert_c_2_1 {
   
	actions 
	{
        
		fffs_c_2_1;
		
    }
	default_action:fffs_c_2_1;
}




table insert_d_1_1 {
   
	actions 
	{
        
		fffs_d_1_1;
		
    }
	default_action:fffs_d_1_1;
}


table insert_d_2_1 {
   
	actions 
	{
        
		fffs_d_2_1;
		
    }
	default_action:fffs_d_2_1;
}


//gettime
action getTime() {
	modify_field(m.tag, 0);
	modify_field_with_hash_based_offset(m.loc1,0,S_1,0);
	modify_field_with_hash_based_offset(m.loc2,0,S_2,0);
	modify_field(m.tstamp, ig_intr_md_from_parser_aux.ingress_global_tstamp);
	
}
table getTimeTable {
	
    actions {getTime;}
    default_action : getTime;
    size: 1;
}

//////////


//set sign
action signing(w)
{
modify_field(m.sign, w);

}
table sign_t
{
reads{m.tag:exact;}
actions{signing;}
default_action:signing;
size:4;
}
////////////



////////////////



action ggs(ggsign)
{
modify_field(m.goodsign1,ggsign);
modify_field(m.goodsign2,ggsign);
modify_field(m.goodsign3,ggsign);
modify_field(m.goodsign4,ggsign);
}
table getgoodsign
{
reads{m.sign:exact;}
actions{ggs;}
default_action:ggs;
size:40;
}

action rm1()
{
modify_field(m.loc5,m.loc1);
modify_field(m.loc6,m.loc2);
modify_field(m.loc7,m.loc3);
modify_field(m.loc8,m.loc4);
modify_field_rng_uniform(m.ramloc, 0, 65535);

}
action rm2()
{
modify_field_rng_uniform(m.ramloc, 0, 65535);
}
action no_op()
{
add_to_field(m.fmin,1);
}
table rdmz
{
reads{m.sign:exact;}
actions{rm1;rm2;no_op;}
default_action:no_op;
size:4;
}



blackbox stateful_alu rrm_a
{
	reg:rm_a;
	update_lo_1_value:1;
	output_value:register_lo;
	output_dst:m.ram1;
}


action rrrm_a()
{
rrm_a.execute_stateful_alu(m.ramloc);
}
table reram1
{
actions{rrrm_a;}
default_action:rrrm_a;
}



action rom1()
{
modify_field_rng_uniform(m.ramloc,0,65535);

}
table random1
{
actions{rom1;}
default_action:rom1;
}



action rs1()
{
modify_field(m.loc1,m.ramloc);
modify_field(m.loc2,m.ramloc);
modify_field(m.loc3,m.ramloc);
modify_field(m.loc4,m.ramloc);
}
action rs2()
{
modify_field(m.loc5,m.ramloc);
modify_field(m.loc6,m.ramloc);
modify_field(m.loc7,m.ramloc);
modify_field(m.loc8,m.ramloc);
}
table restore
{
reads{m.sign:exact;}
actions{rs1;rs2;}
default_action:rs2;
size:4;
}



action write1()
{
modify_field(FS_s.f1,m.fa_1_1);
modify_field(FS_s.f2,m.fb_1_1);
modify_field(FS_s.f3,m.fc_1_1);
modify_field(FS_s.f4,m.fd_1_1);
modify_field(FS_h.sketchid,1);
modify_field(FS_h.binid,m.loc1);
modify_field(FS_h.ID, id);
}
action write2()
{
modify_field(FS_s.f1,m.fa_2_1);
modify_field(FS_s.f2,m.fb_2_1);
modify_field(FS_s.f3,m.fc_2_1);
modify_field(FS_s.f4,m.fd_2_1);
modify_field(FS_h.sketchid,2);
modify_field(FS_h.binid,m.loc5);
modify_field(FS_h.ID, id);
}
table take_t
{
reads{m.sign:exact;}
actions{write1;write2;}
default_action:write1;
size:4;
}


action hashcal_tcp (){
    modify_field(m.dst, ipv4.dip);
    modify_field_with_hash_based_offset(m.hashvalue, 0, hash_tcp, 0);
}
action hashcal_arp (){
    modify_field(m.dst, arp.pDst);
    modify_field_with_hash_based_offset(m.hashvalue, 0, hash_arp, 0);
}
action hashcal_udp (){
    modify_field(m.dst, ipv4.dip);
    modify_field_with_hash_based_offset(m.hashvalue, 0, hash_udp, 0);
}
action hashcal_ipv4 (){
    modify_field(m.dst, ipv4.dip);
    modify_field_with_hash_based_offset(m.hashvalue, 0, hash_ipv4, 0);
}
table hashcal_tcp_t{
    actions{
        hashcal_tcp;
    }
    default_action: hashcal_tcp;
}
table hashcal_arp_t{
    actions{
        hashcal_arp;
    }
    default_action: hashcal_arp;
}
table hashcal_ipv4_t{
    actions{
        hashcal_ipv4;
    }
    default_action: hashcal_ipv4;
}
table hashcal_udp_t{
    actions{
        hashcal_udp;
    }
    default_action: hashcal_udp;
}

action ECMP(egress_port) {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, egress_port);
    modify_field(m.portnum,egress_port);
}

table ECMP_t {
    reads {
        m.dst : exact;
		m.hashvalue: exact;
    }
    actions {
        ECMP;
    }
	default_action:ECMP;
    size: 32;
}

action max_2() {
    max(m.fdt, m.dts1, m.dts2);
}
table wrap_max_2 {
    actions {
        max_2;
    }
    default_action: max_2;
}

action max_1() {
    max(m.fdt, m.fdt, m.dts3);
}
table wrap_max_1 {
    actions {
        max_1;
    }
    default_action: max_1;
}



action locset() {
modify_field_with_hash_based_offset(m.loc3,0,S_3,0);
modify_field_with_hash_based_offset(m.loc4,0,S_4,0);
}
table locset_t {
    actions {
        locset;
    }
    default_action: locset;
}




action getflag() {
modify_field_rng_uniform(m.flag, 0, 15);
}
table getflag_t {
    actions {
        getflag;
    }
    default_action: getflag;
}

control ingress {

if(valid(ipv4)){
        if(valid(tcp)){
            apply(hashcal_tcp_t);
        }
        else if(valid(udp)){
            apply(hashcal_udp_t);
        }
        else{
            apply(hashcal_ipv4_t);
        }
    }
    else if(valid(arp)){
        apply(hashcal_arp_t);
    }
    apply(ECMP_t);
	
if (valid(FS_h))
{

	apply(getTimeTable);
	apply(sign_t);
	apply(c_1);
	apply(c_2);
	apply(wrap_max_2);
	apply(c_3);
	apply(wrap_max_1);
	
	apply(getgoodsign);
	apply(locset_t);
	apply(rdmz);
	apply(getflag_t);
	
if (m.flag==3 and FS_h.ID==0)
{
if (m.sign==1 or m.sign==2)
{
	apply(reram1);
	if (m.ram1==1)
	{
		apply(random1);
	}
}
}

	apply(restore);
}

}
control egress 
{
if (valid(FS_h))
{

	
	
apply(insert_a_1_1);
apply(insert_a_2_1);
apply(insert_b_1_1);
apply(insert_b_2_1);
apply(insert_c_1_1);
apply(insert_c_2_1);
apply(insert_d_1_1);
apply(insert_d_2_1);
	
if (m.flag==3 and FS_h.ID==0)
{
if (m.sign==1 or m.sign==2)
{
apply(take_t);

}
}
}
}	
