import os
import json
import random
import argparse

n_sw = 20
distri = []
flows = []
graph = {}
banned = []

blackhole_flag = False
loop_flag = False
loop_rate = 0.1

'''
PART flowsize
'''
def load_distri(file):
	with open(file, 'r') as f:
		for line in f:
			data = line.strip().split(' ')
			if len(data) < 3:
				break
			distri.append( (float(data[2]), int(data[0])) )


def gen_flowsize():
	mtu = 1460
	rd = random.random()
	for i in range(len(distri)):
		if rd <= distri[i][0]:
			k = (rd-distri[i-1][0]) / (distri[i][0]-distri[i-1][0])
			res = distri[i-1][1] + (distri[i][1]-distri[i-1][i]) * k
			return (res+(mtu-1)) // mtu

def load_flows(file):
	mtu = 1460
	total_pkt_est = 0
	with open(file, 'r') as f:
		for line in f:
			data = line.strip().split(' ')
			if len(data) < 5:
				break
			# data[3] = str((int(data[3])+mtu-1) // mtu)
			total_pkt_est += int(data[3])
			flows.append(data)

	print('total packet est. : ', total_pkt_est)

'''
PART topology
'''
def load_topology(file):
	f = open(file, encoding="utf-8")
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

def find_path(src, dst):
	maxlen = 2
	flag = False
	ans = []
	while not flag and maxlen < 10:
		# print('***********ttl ', maxlen)
		ans = find_path_dfs(src, dst, maxlen)
		if len(ans) > 0:
			flag = True
		maxlen += 1
	return ans


'''
PART generator
'''
def set_banned(u, v):
	banned.append(u+':'+v)
	banned.append(v+':'+u)

def get_ip(host):
	num = int(host[1:])
	ip = '10.'
	ip += str((num-1)//2+13) + '.'
	ip += str(num) + '.'
	ip += '2'
	return ip

def gen_trace(filedir):
	for i in range(1, n_sw+1):
		sw_name = 's' + str(i)
		filename = sw_name + '.log'
		if os.path.exists(filedir + filename):
			os.remove(filedir + filename)
	if os.path.exists('meta.log'):
		os.remove('meta.log')

	for flow in flows:
		src = flow[0]
		dst = flow[1]
		sport = int(flow[2])
		dport = 20000
		flowsize = int(flow[3])
		protocol = int(flow[4])

		identifier = ','.join([get_ip(src), get_ip(dst), str(sport), str(dport), str(protocol)])

		paths = find_path(src, dst)
		k = random.randint(0, len(paths)-1)
		print(identifier)
		print(paths, k)
		path = paths[k][1:-1]

		las = paths[k][0]

		taskflag = 0

		# loop
		if loop_flag and len(path) >= 3:
			rdm = random.random()
			if rdm < loop_rate:		
				taskflag = 1
				loop_len = 2
				loop_point = random.randint(1, len(path)-loop_len)
				path_seg = path[loop_point:loop_point+loop_len]
				new_path = path[:loop_point]
				new_path += list(reversed(path_seg))[1:] + path_seg[1:] 
				new_path += path[loop_point:]
				path = new_path 

		print(path)

		real_path = []
		for sw_name in path:
			# blackhole
			if las+':'+sw_name in banned:
				print('blackhole banned')
				taskflag = 1
				break
			las = sw_name
			real_path.append(sw_name)

			filename = sw_name + '.log'
			pkt = {}
			pkt["switch_name"] = sw_name
			pkt_info = {}
			pkt_info["srcAddr"] = get_ip(src)
			pkt_info["dstAddr"] = get_ip(dst)
			pkt_info["srcPort"] = str(sport)
			pkt_info["dstPort"] = str(dport)
			pkt_info["protocol"] = str(protocol)
			pkt["packet_info"] = pkt_info
			
			with open(filedir+filename, 'a+') as f:
				for i in range(flowsize):
					f.write(str(pkt)+'\n')

		with open('meta.log', 'a+') as f:
			f.write(identifier + '\n' )
			f.write(str(taskflag) + '\n')
			f.write(' '.join(paths[k]) + '\n')
			f.write(' '.join(real_path) + '\n')

def gen_trace_directly(filedir):
	for i in range(1, n_sw+1):
		sw_name = 's' + str(i)
		filename = sw_name + '_bh_100k.dat'
		if os.path.exists(filedir + filename):
			os.remove(filedir + filename)
	if os.path.exists('meta_bh_100k.log'):
		os.remove('meta_bh_100k.log')

	for flow in flows:
		src = flow[0]
		dst = flow[1]
		sport = int(flow[2])
		dport = 20000
		flowsize = int(flow[3])
		protocol = int(flow[4])

		identifier = ','.join([get_ip(src), get_ip(dst), str(sport), str(dport), str(protocol)])

		paths = find_path(src, dst)
		k = random.randint(0, len(paths)-1)
		# print(identifier)
		# print(paths, k)
		path = paths[k][1:-1]

		las = paths[k][0]

		taskflag = 0

		# loop
		if loop_flag and len(path) >= 3:
			rdm = random.random()
			if rdm < loop_rate:		
				taskflag = 1
				loop_len = 2
				num_loop = 10
				loop_point = random.randint(1, len(path)-loop_len)
				path_seg = path[loop_point:loop_point+loop_len]
				new_path = path[:loop_point]
				for i in range(num_loop):
					new_path += list(reversed(path_seg))[1:] + path_seg[1:] 
				new_path += path[loop_point:]
				path = new_path 

		# print(path)

		real_path = []
		for sw_name in path:
			# blackhole
			if las+':'+sw_name in banned:
				# print('blackhole banned')
				taskflag = 1
				break
			las = sw_name
			real_path.append(sw_name)

			filename = sw_name + '_bh_100k.dat'
			srcAddr = get_ip(src)
			dstAddr = get_ip(dst)
			srcPort = str(sport)
			dstPort = str(dport)
			protocol = str(protocol)
			
			with open(filedir+filename, 'a+') as f:
				item = ','.join([srcAddr, dstAddr, srcPort, dstPort, protocol])
				# if item == '10.13.1.2,10.17.9.2,11906,20000,17':
				# 	print(filedir+filename)
				# 	print(sw_name)
				item += ' ' + str(flowsize)
				f.write(item+'\n')

		with open('meta_bh_100k.log', 'a+') as f:
			f.write(identifier + '\n' )
			f.write(str(taskflag) + '\n')
			f.write(' '.join(paths[k]) + '\n')
			f.write(' '.join(real_path) + '\n')

'''
PART preprocess
'''
def preprocess_blackhole():
	global blackhole_flag
	set_banned('s1', 's5')
	set_banned('s12', 's20')
	blackhole_flag = True

def preprocess_loop():
	global loop_flag
	loop_flag = True


if __name__ == '__main__':
	# load_flowsize_distri('./distri.txt')
	load_topology('../../switch/p4src_flowsize/p4app.json')
	load_flows('./flow_100k.txt')
	print(flows)
	preprocess_blackhole()
	gen_trace_directly('../trace/direct/')
