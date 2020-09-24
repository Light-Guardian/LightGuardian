#!/usr/bin/env python
import sys
import os
from scapy.all import *

counter=1;

class SFH(Packet):
    name = 'SFH'
    fields_desc = [\
            BitField('sfh_switch_id',0,16),\
            #BitField('sfh_sketch_fg',0,8),\
            BitField('sfh_fgment_id',0,32),\
            BitField('sfh_delay0',0,32),\
            BitField('sfh_delay1',0,32),\
            BitField('sfh_delay2',0,32),\
            BitField('sfh_delay3',0,32),\
            BitField('sfh_delay4',0,32),\
            BitField('sfh_delay5',0,32),\
            BitField('sfh_delay6',0,32),\
            #BitField('sfh_delay7',0,32),\
            #BitField('sfh_delay8',0,32),\
            BitField('sfh_delay7',0,32)]

class MIH(Packet):
    name="MIH"
    #bitfiled(<name>,<default>,<length>)
    fields_desc=[\
            BitField("mih_switch_id",0,16),\
            BitField("mih_fgment_id",0,32),\

            BitField("mih_padding",0,16),\
            BitField("mih_timestamp",0,48)]

class FLAG(Packet):
    name="flag"
    fields_desc=[\
            BitField("flag",0,8)]

def get_if():
    iface=None
    for i in get_if_list():
        if "eth0" in i:
            iface=i
            break;
    if not iface:
        print "Cannot find eth0 interface"
        exit(1)
    return iface

def isNotOutgoing(my_mac):
    my_mac = my_mac
    def _isNotOutgoing(pkt):
        return pkt[Ether].src != my_mac

    return _isNotOutgoing

def handle_pkt(pkt):
    global counter
    print ("Packet "+str(counter)+" Received:")
    counter+=1
	

    ether = pkt.getlayer(Ether)
    ip = pkt.getlayer(IP)
    tcp=pkt.getlayer(TCP)
    udp=pkt.getlayer(UDP)
    icmp=pkt.getlayer(ICMP)
    if udp:
        flag=FLAG(str(udp.payload))
        
    '''
    print "###[ Ethernet ]###"
    print "  src: {}".format(ether.src)
    print "  dst: {}".format(ether.dst)
    print "###[ IP ]###"
    print "  src: {}".format(ip.src)
    print "  dst: {}".format(ip.dst)
    print "  IP length:{}".format(ip.len)
    '''
    if icmp:
        #print "ICMP packet"
        pass
    
    if udp:
        '''
        print "###[ UDP ]###"
        print "  sport: {}".format(udp.sport)
        print "  dport: {}".format(udp.dport)
        print "  length: {}".format(udp.len)
        print "  chksum: {}".format(udp.chksum)
        print "  udp payload length: {}".format(len(udp.payload))
        '''
        # packet_raw=raw(ip)
        # print len(packet_raw)
        # udp_raw=packet_raw[20:]
        # chksum = in4_chksum(socket.IPPROTO_UDP, pkt[IP], udp_raw)
        # print "scapy chksum:"+str(chksum)
    
        # print "###[ FLAG ]###"
        # print " flag: {}".format(flag.flag)
        # print " flag payload length: {}".format(len(flag.payload))
        
        # if flag.flag&0b1000==0b1000:
        #     mih=MIH(str(flag.payload))
        #     print "###[ MIH ]###"
        #     print "  mih_switch_id: {}".format(mih.mih_switch_id)
        #     print "  mih_timestamp: {}".format(mih.mih_timestamp)
        #     print "  mih_fgment_id: {}".format(mih.mih_fgment_id)
        #     msg = mih.payload

        # elif flag.flag&0b010==0b010: 
        #     sfh=SFH(str(flag.payload))
        #     if (sfh.sfh_fgment_id==64 or sfh.sfh_fgment_id==63 or sfh.sfh_fgment_id==65 ) :
        #         print "###[ SFH ]###"
        #         print "  sfh_switch_id: {}".format(sfh.sfh_switch_id)
        #         #print "  sfh_sketch_fg: {}".format(sfh.sfh_sketch_fg)
        #         print "  sfh_fgment_id: {}".format(sfh.sfh_fgment_id)
        #         print "  sfh_delay0: {}".format(sfh.sfh_delay0)
        #         print "  sfh_delay1: {}".format(sfh.sfh_delay1)
        #         print "  sfh_delay2: {}".format(sfh.sfh_delay2)
        #         print "  sfh_delay3: {}".format(sfh.sfh_delay3)
        #         print "  sfh_delay4: {}".format(sfh.sfh_delay4)
        #         print "  sfh_delay5: {}".format(sfh.sfh_delay5)
        #         print "  sfh_delay6: {}".format(sfh.sfh_delay6)
        #         print "  sfh_delay7: {}".format(sfh.sfh_delay7)
        #     #print "  sfh_delay8: {}".format(sfh.sfh_delay8)
        #     #print "  sfh_delay9: {}".format(sfh.sfh_delay9)
        #     msg = sfh.payload
        # else:
        #     msg=flag.payload
        #print "###[ MESSAGE ]###"
        #
        #print (type(str(msg)))
        #print (msg)
        #print "msg length: {}".format(len(msg))

    #print

def main():
    ifaces = filter(lambda i: 'eth' in i, os.listdir('/sys/class/net/'))
    iface = ifaces[0]
    print "sniffing on %s" % iface
    sys.stdout.flush()
    my_filter = isNotOutgoing(get_if_hwaddr(get_if()))
    sniff(filter="ip", iface = iface,
          prn = lambda x: handle_pkt(x), lfilter=my_filter)

if __name__ == '__main__':
    
    main()
