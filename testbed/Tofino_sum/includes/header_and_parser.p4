
header_type ethernet_t {
    fields {
        dmac : 48;
        smac : 48;
        ethertype : 16;
    }
}
header ethernet_t ethernet;
header ethernet_t ethernet1;


header_type vxlan_t
{
fields
{
vx1:32;
vx2:32;
}
}
header vxlan_t vxlan;



header_type ipv4_t {
    fields {
        ver : 4;
        len : 4;
        diffserv : 6;
        ecn: 2;
        totalLen : 16;
        id : 16;
        flags : 3;
        offset : 13;
        ttl : 8;
        proto : 8;
        csum : 16;
        sip : 32;
        dip : 32;
    }
}
header ipv4_t ipv4;
header ipv4_t ipv41;

field_list ipv4_checksum_list {
        ipv4.ver;
        ipv4.len;
        ipv4.diffserv;
        ipv4.ecn;
        ipv4.totalLen;
        ipv4.id;
        ipv4.flags;
        ipv4.offset;
        ipv4.ttl;
        ipv4.proto;
        ipv4.sip;
        ipv4.dip;
}

field_list_calculation ipv4_checksum {
    input {
        ipv4_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field ipv4.csum  {
    update ipv4_checksum;
}


header_type tcp_t {
    fields {
        sPort : 16;
        dPort : 16;
        seqNo : 32;
        ackNo : 32;
        dataOffset : 4;
        res : 3;
        ecn : 3;
        ctrl : 6;
        window : 16;
        checksum : 16;
        urgentPtr : 16;
    }
}
header tcp_t tcp;

header_type udp_t {
    fields {
        sPort : 16;
        dPort : 16;
        hdr_length : 16;
        checksum : 16;
    }
}
header udp_t udp;
header udp_t udp1;
header_type arp_t{
    fields {
        hType: 16;
        pType: 16;
        hLen: 8;
        pLen: 8;
        oper: 16;
        hSrc: 48;
        pSrc: 32;
        hDst: 48;
        pDst: 32;
    }
}
header arp_t arp;

header_type FS_h_t
{
    fields {
	type:16;
		ID:16;
		binid:16;
		sketchid:16;
		

		
    }
}
header FS_h_t FS_h;
header_type FS_s_t
{
    fields {
        f1:32;
		f2:32;
		f3:32;
		f4:32;
    }
}
header FS_s_t FS_s;

parser start {
    return parse_ethernet;
}

parser parse_ethernet1 {
    extract(ethernet1);
    return select(ethernet1.ethertype) {
        0x0800 : parse_ipv41;
	    0x1000 : parse_FS_h;
        0x0806 : parse_arp;
        default : ingress;
    }
}
parser parse_ipv41 {
    extract(ipv41);
    return select(ipv41.proto) {
        6 : parse_tcp;
        17: parse_udp1;
        default: ingress;
    }
}
parser parse_ipv4 {
    extract(ipv4);
    return select(latest.proto) {
        6 : parse_tcp;
        17: parse_udp;
        default: ingress;
    }
}
parser parse_udp1 {
    extract(udp1);
    return ingress;
}

parser parse_vxlan {
    extract(vxlan);
    return parse_ethernet1;
}
parser parse_ethernet {
    extract(ethernet);
    return select(latest.ethertype) {
        0x0800 : parse_ipv4;
        0x0806 : parse_arp;
        default : ingress;
    }
}


parser parse_FS_h {
    extract(FS_h);
   return parse_FS_s;
    }

parser parse_FS_s {
    extract(FS_s);
   return select(FS_h.type) {
        0x0800 : parse_ipv41;
        0x0806 : parse_arp;
        default : ingress;
    }
    }
	
parser parse_arp{
    extract(arp);
    return ingress;
}
parser parse_tcp {
    extract(tcp);
    return ingress;
}
parser parse_udp {
    extract(udp);
	return select(latest.dPort) {
        4789 : parse_vxlan;
        default: ingress;
    }
    
}


