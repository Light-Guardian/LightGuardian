from parameter import *
import math
import os
import sys
import json

# Parameters for Fat-tree
num_pod = 4
bandwidth = 2



obj = {}
obj['program'] = para_program
obj['switch'] = para_switch
obj['compiler'] = para_compiler
obj['options'] = para_options
obj['switch_cli'] = para_switch_cli
obj['cli'] = True
obj['pcap_dump'] = False
obj['enable_log'] = False

topo_module = {'file_path': '', \
'module_name': 'p4utils.mininetlib.apptopo', \
'object_name': 'AppTopoStrategies'}
obj['topo_module'] = topo_module

obj['controller_module'] = None

topodb_module = {'file_path': '', \
'module_name': 'p4utils.utils.topology', \
'object_name': 'Topology'}
obj['topodb_module'] = topodb_module

mininet_module = {'file_path': '', \
'module_name': 'p4utils.mininetlib.p4net', \
'object_name': 'P4Mininet'}
obj['mininet_module'] = mininet_module

# build network topology
topology = {}
topology['assignment_strategy'] = 'l3'

# Link -  3 levels
# ------------------------
#   k/2  *  k/2         core
#   k/2  *   k          aggregation
#   k/2  *   k          edge
#   k/2 * k * k/2       servers

links = []
half_pod = num_pod//2
core_base = 1
agg_base = core_base + half_pod**2
edge_base = agg_base + half_pod*num_pod

num_switch = edge_base + half_pod*num_pod - core_base
num_host = (half_pod**2) * num_pod

# core -- aggregation
for i in range(half_pod) :
	for j in range(half_pod) :
		core_id = 's' + str(i * half_pod + j + core_base)
		for k in range(num_pod) :
			agg_id = 's' + str(k * half_pod + i + agg_base)
			links.append([core_id, agg_id, {'bw' : bandwidth}])

# aggregation -- edge
for i in range(num_pod) :
	for j in range(half_pod) :
		agg_id = 's' + str(i * half_pod + j + agg_base)
		for k in range(half_pod) :
			edge_id = 's' + str(i * half_pod + k + edge_base)
			links.append([agg_id, edge_id, {'bw' : bandwidth}])

# edge -- servers
for i in range(num_pod) :
	for j in range(half_pod) :
		edge_id = 's' + str(i * half_pod + j + edge_base)
		for k in range(half_pod) :
			sev_id = 'h' + str((i*half_pod+j)*half_pod + k + 1)
			links.append([edge_id, sev_id, {'bw' : bandwidth}])

topology['links'] = links

hosts = {}
for i in range(num_host):
	host_id = 'h' + str(i+1)
	hosts[host_id] = {}
topology['hosts'] = hosts

switches = {}
for i in range(num_switch):
	switch_id = 's' + str(i+1)
	switches[switch_id] = {'cpu_port' : True}
topology['switches'] = switches

obj['topology'] = topology

json_str = json.dumps(obj, indent=4, separators=(',', ': '))
print(json_str)



