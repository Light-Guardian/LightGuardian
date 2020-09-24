import nnpy
import time
import struct
from p4utils.utils.topology import Topology
from p4utils.utils.sswitch_API import SimpleSwitchAPI
from scapy.all import *
import sys
import threading
import argparse


class CPU(Packet):
    name="CPU"
    fields_desc=[\
            BitField('srcAddr',0,32),\
            BitField('dstAddr',0,32),\
            BitField('protocol',0,8),\
            BitField('srcPort',0,16),\
            BitField('dstPort',0,16),\
            BitField('delay',0,48),\
            #BitField('delay2',0,48),\
            BitField('interval',0,48),\
            BitField('flags',0,8)]


class packetReceicer(threading.Thread):

    def __init__(self, sw_name,program):
        threading.Thread.__init__(self)
        if program=="f":
            self.topo = Topology(db="../p4src_flowsize/topology.db")  #set the topology
        elif program=="i":
            self.topo = Topology(db="../p4src_interval/topology.db")  #set the topology
        self.sw_name = sw_name
        self.thrift_port = self.topo.get_thrift_port(sw_name)
        self.cpu_port =  self.topo.get_cpu_port_index(self.sw_name)
        self.controller = SimpleSwitchAPI(self.thrift_port)
        self.flow={}
        self.flag=True
        self.init()
        

    def init(self):
        self.add_mirror()
        self.counter=1
        self.logs=open("../switch_log/"+self.sw_name+".log","w")
        self.logs_info=open("../switch_log/"+self.sw_name+"_info.log","w")
        self.logs_info.write("SWITCH["+self.sw_name+"]\n")
        self.logs.close()
        self.logs_info.close()

    def add_mirror(self):
        if self.cpu_port:
            self.controller.mirroring_add(100, self.cpu_port) # correspond to the 100 in p4 code
            #is there any probability to increase the mirro port to add cpu port?

    def recv_msg_cpu(self, pkt):
        ## console output starts
        #print
        #print("["+self.sw_name+"] received packet number:"+str(self.counter))
        self.counter+=1
        cpu=CPU(str(pkt))
        #ls(cpu)

        ## console output ends
        type=(cpu.flags>>2);
        if self.flag==True:
            logs=open("../switch_log/"+self.sw_name+".log","w")
            self.flag=False
            if type==0:
                logs.write("flowsize information collecting\n")
            else:
                logs.write("interval information collecting\n")
            
            logs.close()

        self.gen_per_packet_log(cpu)
        self.collect_log(cpu)
        if(self.counter%1000==0):
            self.gen_log()

    def gen_log(self):
        logs_info=open("../switch_log/"+self.sw_name+"_info.log","a")
        logs_info.write("[flow number: "+str(len(self.flow))+"]\n")
        change=lambda x: '.'.join([str(x/(256**i)%256) for i in range(3,-1,-1)])

        cnt=0
        for i in self.flow:
            cnt+=self.flow[i]["packnum"]
            tmp=i.split(":")
            tmp[0]=change(int(tmp[0]))
            tmp[1]=change(int(tmp[1]))
            tmp=" : ".join(tmp)
            logs_info.write("flow "+tmp+" ")

            logs_info.write(str(sorted(self.flow[i].items())))
            logs_info.write("\n")
        logs_info.write("[packet number sum:"+str(cnt)+"]\n\n")

        logs_info.close()   
    
    def collect_log(self,cpu):
        flow_key=str(cpu.srcAddr)+":"+str(cpu.dstAddr)+":"+str(cpu.protocol)+":"+str(cpu.srcPort)+":"+str(cpu.dstPort)
        if self.flow.has_key(flow_key):
            self.flow[flow_key]["packnum"]+=1
            self.flow[flow_key][self.get_lev(cpu.delay)]+=1
        else:
            self.flow[flow_key]={"packnum":1,"0->1":0,"1->2":0,\
                "2->3":0,"3->4":0,"4->5":0,"5->6":0,"6->7":0\
                ,"7+":0}#"7->8":0,"8->9":0,"9+":0}
            self.flow[flow_key][self.get_lev(cpu.delay)]+=1
            
    def get_lev(self,delay):
        time_interval=1000
        if delay<time_interval*1:
            return "0->1"
        elif delay<time_interval*2:
            return "1->2"
        elif delay<time_interval*3:
            return "2->3"
        elif delay<time_interval*4:
            return "3->4"
        elif delay<time_interval*5:
            return "4->5"
        elif delay<time_interval*6:
            return "5->6"
        elif delay<time_interval*7:
            return "6->7"
        # elif delay<time_interval*8:
        #     return "7->8"
        # elif delay<time_interval*9:
        #     return "8->9"
        else:
            return "7+"


    def gen_per_packet_log(self,cpu):
        logs=open("../switch_log/"+self.sw_name+".log","a")
        change=lambda x: '.'.join([str(x/(256**i)%256) for i in range(3,-1,-1)])
        
        srcAddr=change(cpu.srcAddr)
        dstAddr=change(cpu.dstAddr)
        tmp_delay=str(cpu.delay)
        delay=tmp_delay[-9:-6]+"s "+tmp_delay[-6:-3]+"ms "+tmp_delay[-3:]+"us"
        tmp_interval=str(cpu.interval)
        interval=tmp_interval[-9:-6]+"s "+tmp_interval[-6:-3]+"ms "+tmp_interval[-3:]+"us"
        sketch_fg=(cpu.flags>>1)&0x1;
        has_SFH=cpu.flags&0x1;
        type=(cpu.flags>>2)&0x1;

        
        logs.write('{"switch name":"'+self.sw_name+'",')
        logs.write('"packet number":"'+str(self.counter-1)+'","packet_info":{')
        logs.write('"srcAddr":"'+str(srcAddr)+'",')
        logs.write('"dstAddr":"'+str(dstAddr)+'",')
        logs.write('"protocol":"'+str(cpu.protocol)+'",')
        logs.write('"srcPort":"'+str(cpu.srcPort)+'",')
        logs.write('"dstPort":"'+str(cpu.dstPort)+'",')
        logs.write('"delay ":"'+delay+'",')
        logs.write('"interval":"'+interval)
        logs.write('"timestamp":'+str(time.time()))
        if type==0:
            logs.write('",'+'"using sketch":"'+str(sketch_fg)+'",')
            logs.write('"bring SFH":'+str(bool(has_SFH)))
        else :
            logs.write('",'+'"using sketch":"'+str(sketch_fg)+'",')
            logs.write('"bring MIH":'+str(bool(has_SFH)))
        logs.write(" }}\n")
        logs.close()


    

    def run_cpu_port_loop(self):
        cpu_port_intf = str(self.topo.get_cpu_port_intf(self.sw_name).replace("eth0", "eth1"))
        #the cpu has two ports   could use two thread to sniff
        print(cpu_port_intf)
        print
        print(sniff(iface=cpu_port_intf, prn=self.recv_msg_cpu))
    
    def run(self):
        self.run_cpu_port_loop()


if __name__ == "__main__":
    #usage : python test.py [sw_name]
    #hint: the sw_name must open the cpu_port function in p4app.json
    #"cpu_port" : true

    parser=argparse.ArgumentParser()
    
    parser.add_argument("-s","--switch",help="this switch's name")
    parser.add_argument("p",help="the program to be run",choices=["f","i"])
    args=parser.parse_args()
    
    controllers = []

    if args.switch==None:
        num_switch = 20

        for i in range(num_switch):
            controllers.append(packetReceicer("s"+str(i+1),args.p))

        for i in range(num_switch):
            controllers[i].start()
        
        for i in range(num_switch):
            controllers[i].join()

    else:
        sw_name = args.switch
        controllers.append(packetReceicer(sw_name,args.p).run_cpu_port_loop())
