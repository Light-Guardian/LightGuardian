from __future__ import print_function

from p4utils.utils.topology import Topology
from p4utils.utils.sswitch_API import *

import threading
import time
import argparse

BUCKET_NUM = 64
BIN_NUM = 8
ARRAY_NUM = 3

UPDATE_INTERVAL = 600

lock = threading.Lock()

class Monitor(threading.Thread):

	def __init__(self, sw_name,program):
		threading.Thread.__init__(self)
		if program=="f":
			self.topo = Topology(db="../p4src_flowsize/topology.db")  #set the topology
		elif program=="i":
			self.topo = Topology(db="../p4src_interval/topology.db")  #set the topology
		
		self.program=program
		self.sw_name = sw_name
		self.thrift_port = self.topo.get_thrift_port(sw_name)
		self.controller = SimpleSwitchAPI(self.thrift_port)



	def sketch_swap(self):
		print ("start swapping sketch for switch "+self.sw_name)
		self.controller.register_write("swap_control",0,1)
	
		present_sketch=self.controller.register_read("sketch_fg",0)
		
		if present_sketch == 0 :
			if self.program=="f":
				self.controller.register_write("array3",[0,BUCKET_NUM * BIN_NUM],0)
				self.controller.register_write("array4",[0,BUCKET_NUM * BIN_NUM],0)
				self.controller.register_write("array5",[0,BUCKET_NUM * BIN_NUM],0)
			elif self.program=="i":
				self.controller.register_write("max_interval_array3",[0,BUCKET_NUM],0)
				self.controller.register_write("max_interval_array4",[0,BUCKET_NUM],0)
				self.controller.register_write("max_interval_array5",[0,BUCKET_NUM],0)


		else :
			if self.program=="f":
				self.controller.register_write("array0",[0,BUCKET_NUM * BIN_NUM],0)
				self.controller.register_write("array1",[0,BUCKET_NUM * BIN_NUM],0)
				self.controller.register_write("array2",[0,BUCKET_NUM * BIN_NUM],0)
			elif self.program=="i":
				self.controller.register_write("max_interval_array0",[0,BUCKET_NUM],0)
				self.controller.register_write("max_interval_array1",[0,BUCKET_NUM],0)
				self.controller.register_write("max_interval_array2",[0,BUCKET_NUM],0)


		self.controller.register_write("counter0", [0,BUCKET_NUM * ARRAY_NUM], 0);
		self.controller.register_write("counter1", [0,BUCKET_NUM * ARRAY_NUM], 0);
		self.controller.register_write("counter2", [0,BUCKET_NUM * ARRAY_NUM], 0);


		self.controller.register_write("swap_control",0,0)
		self.controller.register_write("sketch_fg",0,1-present_sketch)
		print ("end swapping sketch")
		print ("currently using sketch "+ str(1-present_sketch)+" to record")
		print ("currently using sketch "+ str(present_sketch)+" to bring\n")


		#lock.acquire()
		#self.visor()
		#lock.release()

		#time.sleep(UPDATE_INTERVAL - 2)
	
	def visor(self):
		sf = self.controller.register_read("sketch_fg", 0)
		print("switch id :{}".format(self.sw_name))
		print("sketch flag: {}".format(sf))
		
		if sf == 1:
			print("array0:")
			for j in range(0, 10):
				for i in range(0, 16):
					value = self.controller.register_read("array0", i * 10 + j)
					print("|{:^3}|".format(value), end = "")
				print()

			print("array1:")
			for j in range(0, 10):
				for i in range(0, 16):
					value = self.controller.register_read("array1", i * 10 + j)
					print("|{:^3}|".format(value), end = "")
				print()

			print("array2:")
			for j in range(0, 10):
				for i in range(0, 16):
					value = self.controller.register_read("array2", i * 10 + j)
					print("|{:^3}|".format(value), end = "")
				print()
		else:
			print("array3:")
			for j in range(0, 10):
				for i in range(0, 16):
					value = self.controller.register_read("array3", i * 10 + j)
					print("|{:^3}|".format(value), end = "")
				print()

			print("array4:")
			for j in range(0, 10):
				for i in range(0, 16):
					value = self.controller.register_read("array4", i * 10 + j)
					print("|{:^3}|".format(value), end = "")
				print()

			print("array5:")
			for j in range(0, 10):
				for i in range(0, 16):
					value = self.controller.register_read("array5", i * 10 + j)
					print("|{:^3}|".format(value), end = "")
				print()

		# for i in range(0, 16):
		#     value = self.controller.register_read("timestamp_array0", i)
		#     print("|{:^16}|".format(value), end = "")
		#     value = self.controller.register_read("timestamp_array1", i)
		#     print("|{:^16}|".format(value), end = "")
		#     value = self.controller.register_read("timestamp_array2", i)
		#     print("|{:^16}|".format(value), end = "")

	def monitor(self):
		while True:
			current_time = self.controller.sswitch_client.get_time_elapsed_us() \
					% (UPDATE_INTERVAL * 1000 * 1000)
			if current_time < 1 * 1000 * 1000:
				print(self.controller.sswitch_client.get_time_elapsed_us())
				self.sketch_swap()

	def run(self):
		# print("monitor switch {}".format(self.sw_name))
		self.sketch_swap()

if __name__ == "__main__":
	parser=argparse.ArgumentParser()
	parser.add_argument("p",help="the program to be run",choices=["f","i"])
	args=parser.parse_args()
	'''
	Monitor("s1",args.p).sketch_swap()
	Monitor("s2",args.p).sketch_swap()
	Monitor("s3",args.p).sketch_swap()
	Monitor("s4",args.p).sketch_swap()
	Monitor("s5",args.p).sketch_swap()
	Monitor("s6",args.p).sketch_swap()
	Monitor("s7",args.p).sketch_swap()
	Monitor("s8",args.p).sketch_swap()
	Monitor("s9",args.p).sketch_swap()
	Monitor("s10",args.p).sketch_swap()
	'''
	num_switch = 20
	monitors = []

	for i in range(num_switch):
		monitors.append(Monitor("s"+str(i+1),args.p))

	for i in range(num_switch):
		monitors[i].start()

	for i in range(num_switch):
		monitors[i].join()
