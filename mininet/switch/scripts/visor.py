from __future__ import print_function

from p4utils.utils.topology import Topology
from p4utils.utils.sswitch_API import *
import socket, struct, pickle, os
from scapy.all import Ether, sniff, Packet, BitField
import time
import sys

class Monitor(object):

    def __init__(self, sw_name):

        self.topo = Topology(db="../p4src_flowsize/topology.db")
        self.sw_name = sw_name
        self.thrift_port = self.topo.get_thrift_port(sw_name)
        self.controller = SimpleSwitchAPI(self.thrift_port)

    def visor(self):
        sf = self.controller.register_read("sketch_fg", 0)
        print("sketch flag: {}".format(sf))
        
        if sf == 0:
            print("array0:")
            for j in range(0, 8):
                for i in range(0, 64):
                    value = self.controller.register_read("array0", i * 8 + j)
                    print("|{:^3}|".format(value), end = "")
                print()
                print()

            print("array1:")
            for j in range(0, 8):
                for i in range(0, 64):
                    value = self.controller.register_read("array1", i * 8 + j)
                    print("|{:^3}|".format(value), end = "")
                print()
                print()

            print("array2:")
            for j in range(0, 8):
                for i in range(0, 64):
                    value = self.controller.register_read("array2", i * 8 + j)
                    print("|{:^3}|".format(value), end = "")
                print()
                print()
        else:
            print("array3:")
            for j in range(0, 8):
                for i in range(0, 64):
                    value = self.controller.register_read("array3", i * 8 + j)
                    print("|{:^3}|".format(value), end = "")
                print()
                print()

            print("array4:")
            for j in range(0, 8):
                for i in range(0, 64):
                    value = self.controller.register_read("array4", i * 8 + j)
                    print("|{:^3}|".format(value), end = "")
                print()
                print()

            print("array5:")
            for j in range(0, 8):
                for i in range(0, 64):
                    value = self.controller.register_read("array5", i * 8 + j)
                    print("|{:^3}|".format(value), end = "")
                print()
                print()

        # for i in range(0, 16):
        #     value = self.controller.register_read("timestamp_array0", i)
        #     print("|{:^16}|".format(value), end = "")
        #     value = self.controller.register_read("timestamp_array1", i)
        #     print("|{:^16}|".format(value), end = "")
        #     value = self.controller.register_read("timestamp_array2", i)
        #     print("|{:^16}|".format(value), end = "")
        #     print()


if __name__ == "__main__":
    Monitor(sys.argv[1]).visor()
