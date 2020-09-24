import nnpy
import struct
from p4utils.utils.topology import Topology
from p4utils.utils.sswitch_API import SimpleSwitchAPI
import sys
import argparse
import random

class GenFault(object):
    def __init__(self,program):
        if program=="f":
            self.topo = Topology(db="../p4src_flowsize/topology.db")  #set the topology
        elif program=="i":
            self.topo = Topology(db="../p4src_interval/topology.db")  #set the topology
        self.controllers = {}                   #the switches
        self.init()

    def init(self):
        self.connect_to_switches()              

    def connect_to_switches(self):
        for p4switch in self.topo.get_p4switches():
            thrift_port = self.topo.get_thrift_port(p4switch) 
            self.controllers[p4switch] = SimpleSwitchAPI(thrift_port)

    def loop(self):
        switches=raw_input("type the switch's name to gen loop,seperated by ','\nmust be physically loop-able:\n").split(',')
        
        IPs=[]
        for sw_name in self.controllers.keys():
            for host in self.topo.get_hosts_connected_to(sw_name):
                host_ip = self.topo.get_host_ip(host) + "/24"
                IPs.append(host_ip)
        
        for i in range(len(switches)):
            sw_name=switches[i]
            self.controllers[sw_name].table_clear("ecmp_group_to_nhop")
            self.controllers[sw_name].table_clear("ipv4_lpm")
            
            #next_hop=NULL
            if i==len(switches)-1:
                next_hop=switches[0]
            else:
                next_hop=switches[i+1]

            sw_port = self.topo.node_to_node_port_num(sw_name, next_hop)
            dst_sw_mac = self.topo.node_to_node_mac(next_hop, sw_name)
            #print "table_add at {}:".format(sw_name)
            for host_ip in IPs:
                self.controllers[sw_name].table_add("ipv4_lpm", "set_nhop", [str(host_ip)],\
                                [str(dst_sw_mac), str(sw_port)])


    def blackhole(self,args):
        if args.sw_name== None:
            pass
            print "Not implemented yet,please specify the switch name"
        else:
            self.controllers[args.sw_name].table_clear("ecmp_group_to_nhop")
            self.controllers[args.sw_name].table_clear("ipv4_lpm")
            print args.sw_name,"has been shut down"


    def remove_cpu(self):
        # log=open("./router.log","w")
        # log.write(str(self.topo))
        
        print (1)
        print (self.topo.get_shortest_paths_between_nodes("s5","h2"))
        # print(self.topo["sw-cpu"])
        # print(self.topo.network_graph["sw-cpu"])
        
        self.topo.network_graph.remove_node("sw-cpu")
        # self.topo.save("../p4src_interval/topology.db")
        # self.topo.load("../p4src_interval/topology.db")
        # del self.topo
        #self.topo=Topology(db="../p4src_interval/topology.db")
        print ("\n\n\n\n\n")

        print (2)
        print (self.topo.get_shortest_paths_between_nodes("h1","h8"))

        # print(self.topo["sw-cpu"])
        # print(self.topo.network_graph["sw-cpu"])

        # log=open("./router1.log","w")
        # log.write(str(self.topo))
        

    def reroute(self):
        #log=open("./router.log","w")
        #log.write(str(self.topo))
        self.topo.network_graph.remove_node("sw-cpu")
        switch_ecmp_groups = {sw_name:{} for sw_name in self.topo.get_p4switches().keys()}
        for sw_name, controllers in self.controllers.items():
            controllers.table_clear("ecmp_group_to_nhop")
            controllers.table_clear("ipv4_lpm")
            for sw_dst in self.topo.get_p4switches():
                #if its ourselves we create direct connections
                if sw_name == sw_dst:
                    for host in self.topo.get_hosts_connected_to(sw_name):
                        sw_port = self.topo.node_to_node_port_num(sw_name, host)
                        host_ip = self.topo.get_host_ip(host) + "/32"
                        host_mac = self.topo.get_host_mac(host)

                        #add rule
                        print "table_add at {}:".format(sw_name)
                        # log.write("[1] table_add ipv4_lpm set_nhop at {} to host {} using port {}\n".format(sw_name,host,sw_port))
                        self.controllers[sw_name].table_add("ipv4_lpm", "set_nhop", [str(host_ip)], [str(host_mac), str(sw_port)])

                #check if there are directly connected hosts
                else:
                    if self.topo.get_hosts_connected_to(sw_dst):
                        paths = self.topo.get_shortest_paths_between_nodes(sw_name, sw_dst)
                        for host in self.topo.get_hosts_connected_to(sw_dst):

                            if len(paths) == 1:
                                next_hop = paths[0][1]

                                host_ip = self.topo.get_host_ip(host) + "/24"
                                sw_port = self.topo.node_to_node_port_num(sw_name, next_hop)
                                dst_sw_mac = self.topo.node_to_node_mac(next_hop, sw_name)

                                #add rule
                                print "table_add at {}:".format(sw_name)
                                # log.write("[2] table_add ipv4_lpm set_nhop at {} to host {} using port {} to nexthop {}\n".format(sw_name,host,sw_port,next_hop))
                                self.controllers[sw_name].table_add("ipv4_lpm", "set_nhop", [str(host_ip)],
                                                                    [str(dst_sw_mac), str(sw_port)])

                            elif len(paths) > 1:
                                next_hops = [x[1] for x in paths]
                                dst_macs_ports = [(self.topo.node_to_node_mac(next_hop, sw_name),
                                                   self.topo.node_to_node_port_num(sw_name, next_hop))
                                                  for next_hop in next_hops]
                                host_ip = self.topo.get_host_ip(host) + "/24"

                                #check if the ecmp group already exists. The ecmp group is defined by the number of next
                                #ports used, thus we can use dst_macs_ports as key
                                if switch_ecmp_groups[sw_name].get(tuple(dst_macs_ports), None):
                                    ecmp_group_id = switch_ecmp_groups[sw_name].get(tuple(dst_macs_ports), None)
                                    print "table_add at {}:".format(sw_name)
                                    # log.write("[3] table_add ipv4_lpm ecmp_group at {} to switch {} to paths{}\n".format(sw_name,sw_dst,paths))
                                    self.controllers[sw_name].table_add("ipv4_lpm", "ecmp_group", [str(host_ip)],
                                                                        [str(ecmp_group_id), str(len(dst_macs_ports))])

                                #new ecmp group for this switch
                                else:
                                    new_ecmp_group_id = len(switch_ecmp_groups[sw_name]) + 1
                                    switch_ecmp_groups[sw_name][tuple(dst_macs_ports)] = new_ecmp_group_id

                                    #add group
                                    for i, (mac, port) in enumerate(dst_macs_ports):
                                        print "table_add at {}:".format(sw_name)
                                        #log.write("[4] table_add ipv4_lpm ecmp_group at {} to switch {} to paths{}\n".format(sw_name,sw_dst,paths))
                                        # log.write("[4] table_add ipv4_lpm ecmp_group at {} to switch {} using port {}\n".format(sw_name,sw_dst,port))
                                        self.controllers[sw_name].table_add("ecmp_group_to_nhop", "set_nhop",
                                                                            [str(new_ecmp_group_id), str(i)],
                                                                            [str(mac), str(port)])

                                    #add forwarding rule
                                    print "table_add at {}:".format(sw_name)
                                    # log.write("[5] table_add ipv4_lpm ecmp_group at {} to switch {} to paths{}\n".format(sw_name,sw_dst,paths))

                                    self.controllers[sw_name].table_add("ipv4_lpm", "ecmp_group", [str(host_ip)],
                                                                        [str(new_ecmp_group_id), str(len(dst_macs_ports))])
		

    

if __name__ == "__main__":
    parser=argparse.ArgumentParser()
    parser.add_argument("type",help="the type wanted for the net",choices=["loop","blackhole","reset","test"],default="reset")
    parser.add_argument("p",help="the program to be run",choices=["f","i"])

    #group = parser.add_mutually_exclusive_group()
    #group.add_argument("-n","--number",help="the loop's length or the number for blackhole",type=int,default=1)
    parser.add_argument("-s","--sw_name",help="specify the blackhole switch")
    
    args=parser.parse_args()
    fault=GenFault(args.p)

    if args.type=="loop":
        fault.loop()
    elif args.type=="blackhole":
        fault.blackhole(args)
    elif args.type=="reset":
        fault.reroute()
    else:
        print "test start"
        fault.remove_cpu()
        print "test end"
