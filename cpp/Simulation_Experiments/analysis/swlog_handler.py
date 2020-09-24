import os
import json

from parameter import *

global_idf = {}
sw_flows = {}

'''
input the map of switch name (switch -> client)
'''
def read_swmap():
	
	swmap = {}
	with open(swmap_file, 'r') as f:
		for line in f:
			data = line.split()
			swmap[ data[1] ] = data[0] # example: swmap['s1'] = 9
	print(swmap)
	return swmap

'''
handle the output switch logs and generate traces
'''
def handle_swlog():

	for i in range(1, num_switch+1):
		sname = 's' + str(i)
		swlog_file = swlog_dir + sname + '.log'
		trace = []
		flows = {}

		with open(swlog_file, 'r') as f:
			flag = True
			cnt = 0

			for line in f:
				# if flag:
				# 	flag = False
				# 	continue

				# preprocess
				# line = line.replace('"True', 'True')
				# line = line.replace('"False', 'False')
				# line = line.replace('"packet_info', '"packet_info"')
				# line = line.replace('timestamp":', 'timestamp":"')
				data = eval(line)
				pkt = data['packet_info']
				# print(data)

				# cnt += 1
				# if cnt >= 10:
				# 	break

				# if pkt['using sketch'] == '1' :
				# 	break

				identifier = ','.join([pkt['srcAddr'], pkt['dstAddr'], pkt['srcPort'], pkt['dstPort'], pkt['protocol']])
				# print(identifier)
				trace.append(identifier)

				if identifier not in flows:
					flows[identifier] = 0
					global_idf[identifier] = True
				flows[identifier] += 1

		trace_name = sname + output_suffix
		with open(trace_dir + trace_name, 'w') as f:
			for item in trace:
				f.write(item+'\n')

		sw_flows[sname] = flows

interval_changed = {}

'''
handle the output switch logs and generate traces (interface)
'''
def handle_swlog_iface():

	for i in range(1, num_switch+1):

		flows = {}
		sname = 's' + str(i)
		flows_port = [{} for i in range(6)]
		use_port = []
		sum_flow = 0

		for j in range(1, num_eth+1):
			ifname = sname + '_eth' + str(j)
			swlog_file = swlog_iface_dir + ifname + '.log'

			thissw_flows = {}

			if not os.path.exists(swlog_file):
				continue

			with open(swlog_file, 'r') as f:
				flag = True
				cnt = 0

				for line in f:
					if flag:
						flag = False
						continue

					# preprocess
					line = line.replace(' }}', '"}}')
					line = line.replace(',"ingress', '","ingress')
					data = eval(line)
					pkt = data['packet_info']
					ingress = pkt['ingress timestamp']
					# print(data)

					identifier = ','.join([pkt['srcAddr'], pkt['dstAddr'], pkt['srcPort'], pkt['dstPort'], pkt['protocol']])
					# print(identifier)
					# trace.append(identifier)

					if identifier not in flows:
						flows[identifier] = 0
						global_idf[identifier] = True

					if identifier not in flows_port[j]:
						flows_port[j][identifier] = []
					thissw_flows[identifier] = True
					flows[identifier] += 1
					sum_flow += 1
					flows_port[j][identifier].append(ingress)

			if j == 5:
				for k,_ in thissw_flows.items():
					interval_changed[k] = True

		# output trace
		print_line = 0
		trace_name = sname + output_suffix
		with open(trace_dir + trace_name, 'w') as f:
			trace = []
			for k,_ in flows.items():
				use_port = []
				for j in range(num_eth+1):
					if k in flows_port[j]:
						use_port.append(j)

				if len(use_port) != 2:
					print(use_port)
				assert len(use_port)==2
				outport = use_port[1] if int(flows_port[use_port[0]][k][0]) < int(flows_port[use_port[1]][k][0]) else use_port[0]

				for stamp in flows_port[outport][k]:
					print_line += 1
					# item = k + ' ' + stamp;
					trace.append((stamp, k))

			trace = sorted(trace)
			for stamp,k in trace:
				f.write(k + ' ' + stamp + '\n')

		print(sum_flow/2, print_line)

		sw_flows[sname] = flows


'''
check the sum of download sketches
'''
def sum_dlsk():

	for i in range(1, num_switch+1):
		sname = 's' + str(i)
		dlsk_name = 'switch' + swmap[sname] + '_array0.txt'
		dlsk_dir = './download_sketch/'

		res = 0
		with open(dlsk_dir + dlsk_name, 'r') as f:
			for line in f:
				data = line.strip().split()
				for item in data:
					res += int(item)
		print(res)


'''
print routing path for a specific flow
'''
def print_path(identifier):
	# srcAddr = '10.15.5.2'
	# dstAddr = '10.14.3.2'
	# srcPort = '58667'
	# dstPort = '19783'
	# protocol = '17'
	# identifier = ','.join([srcAddr, dstAddr, srcPort, dstPort, protocol])
	# identifier = '10.13.2.2,10.20.15.2,51486,20205,17'

	exist_sw = []
	for i in range(1, num_switch+1):
		sname = 's' + str(i)
		if identifier in sw_flows[sname]:
			exist_sw.append(sname)

	print(exist_sw)
	return exist_sw

path_meta = {}

def load_path_meta():
	cnt = 0
	taskflag = 0
	with open(meta_file, 'r') as f:
		for line in f:
			line = line.strip()
			if cnt % 4 == 0:
				identifier = line
			elif cnt % 4 == 1:
				taskflag = bool(line)
			elif cnt % 4 == 2:
				pass
			elif cnt % 4 == 3:
				path_meta[identifier] = line.split(' ')
		cnt += 1

'''
print routing path in meta file
'''
def print_path_meta(identifier):
	return path_meta[identifier]

graph = {}

def load_topology(file):
	f = open(file)
	cfg = json.load(f)

	for i in range(1, 17):
		graph["h"+str(i)] = []
	for i in range(1, 21):
		graph["s"+str(i)] = []

	links = cfg["topology"]["links"]
	for link in links:
		u = link[0]
		v = link[1]
		graph[u].append(v)
		graph[v].append(u)

	print(graph)

def find_path_dfs(u, dst, ttl):
	# print(u, dst, ttl)
	if ttl == 0:
		if u == dst:
			return [[dst]]
		else:
			return []

	ans = []
	for v in graph[u]:
		res = find_path_dfs(v, dst, ttl-1)
		ans.extend(res)
	for i in range(len(ans)):
		ans[i].insert(0, u)
	# print(ans)
	return ans

def create_meta():
	with open('meta_10k.txt', 'w') as f:
		for k,_ in global_idf.items():
			items = k.split(',')
			src_id = items[0].split('.')[-2]
			dst_id = items[1].split('.')[-2]
			exist_sw = print_path(k)
			real_len = len(exist_sw) + 1
			real_paths = find_path_dfs('h'+src_id, 'h'+dst_id, real_len)

			flag = False
			ans = []
			for path in real_paths:
				mid_path = path[1:-1]
				node_all_exist = True
				for node in mid_path:
					if node not in exist_sw:
						node_all_exist = False
						break
				if node_all_exist:
					flag = True
					ans = path
					break

			assert flag

			f.write(k + '\n')
			change_point = 0
			if k in interval_changed:
				for i in range(1, len(ans)-1):
					if ans[i] == 's1' and ans[i+1] == 's9':
						change_point = 9
						break
					elif ans[i] == 's9' and ans[i+1] == 's1':
						change_point = 1
						break
					elif ans[i] == 's4' and ans[i+1] == 's6':
						change_point = 6
						break
					elif ans[i] == 's6' and ans[i+1] == 's4':
						change_point = 4
						break

			f.write(str(change_point) + '\n')
			f.write(' '.join(ans) + '\n')
			f.write('just placeholder\n')



'''
check switch log for each flow, show the drop result 
'''
def check_drop():
	overall = True
	for identifier, _ in global_idf.items():
		print(identifier)
		exist_sw = print_path(identifier)
		cnt = [(sw_flows[sw_name][identifier], sw_name) for sw_name in exist_sw]
		print(cnt)
		flag = True
		for item in cnt:
			if item[0] != cnt[0][0]:
				flag = False
				break
		overall |= flag
		print(flag)
		print('')
	print('overall ' + str(overall))

if __name__ == '__main__':
	swmap = read_swmap()
	handle_swlog_iface()
	# load_path_meta()
	# print_path()
	check_drop()
	load_topology('../../switch/p4src_flowsize/p4app.json')
	create_meta()
	# sum_dlsk()
