import importlib
import os
import datetime
import time
from thrift.transport import TSocket, TTransport
from thrift.protocol import TBinaryProtocol, TMultiplexedProtocol

p4_name = 'timeSF'

import sys
sys.path.append(os.environ["SDE_INSTALL"] + "/lib/python2.7/site-packages/tofinopd/" + p4_name)
sys.path.append(os.environ["SDE_INSTALL"] + "/lib/python2.7/site-packages/tofino")
sys.path.append(os.environ["SDE_INSTALL"] + "/lib/python2.7/site-packages")

from res_pd_rpc.ttypes import *
from tm_api_rpc.ttypes import *
from p4_pd_rpc.ttypes import *
from ptf.thriftutils import *

from pal_rpc.ttypes import *
iphash2port_conf = open("ecmp7.txt")
iphash2port = []
for t in iphash2port_conf:
    d = t.split()
    iphash2port.append((d[0], int(d[1]),int(d[2])))
	

r_conf=open("ggs.txt")
r=[]
for t in r_conf:
    d = t.split()
    r.append((int(d[0]), int(d[1])))  	

    
def connect():
    switchtime=118
    totaltime=120
    p4_client_module = importlib.import_module("p4_pd_rpc." + p4_name)
    transport = TSocket.TSocket("localhost", 9090)
    bprotocol = TBinaryProtocol.TBinaryProtocol(transport)

    p4_protocol = TMultiplexedProtocol.TMultiplexedProtocol(bprotocol, p4_name)
    client = p4_client_module.Client(p4_protocol)
    pal_protocol = TMultiplexedProtocol.TMultiplexedProtocol(bprotocol, "pal")
    pal_client_module = importlib.import_module(".".join(["pal_rpc", "pal"]))
    conn_mgr_client_module = importlib.import_module('.'.join(['conn_mgr_pd_rpc', 'conn_mgr']))
    conn_mgr_protocol = TMultiplexedProtocol.TMultiplexedProtocol(bprotocol, 'conn_mgr')
    conn_mgr = conn_mgr_client_module.Client(conn_mgr_protocol)

    transport.open()
	
    pal = pal_client_module.Client(pal_protocol)
    dev_id = 0
    ports = [0, 4, 8, 12, 16, 20, 24, 28,32,36,40,44,48,52,56,60]
    pal.pal_port_del_all(dev_id)
    for port in ports:
        pal.pal_port_add(dev_id, port, pal_port_speed_t.BF_SPEED_40G, pal_fec_type_t.BF_FEC_TYP_NONE)
    pal.pal_port_enable_all(dev_id)
    sess_hdl = conn_mgr.client_init()
    dev_tgt = DevTarget_t(0, int(0xFFFF) - 0x10000)



    for kv in iphash2port:
        match_spec = timeSF_ECMP_t_match_spec_t(ipv4Addr_to_i32(kv[0]),kv[1])
        action_spec = timeSF_ECMP_action_spec_t(kv[2])
        try:
            client.ECMP_t_table_delete_by_match_spec(sess_hdl, dev_tgt, match_spec)
        except:
            pass
        client.ECMP_t_table_add_with_ECMP(sess_hdl, dev_tgt, match_spec, action_spec)



    pr=255
    match_spec = timeSF_take_t_match_spec_t(2)
    client.take_t_table_add_with_write2(sess_hdl, dev_tgt, match_spec)
    match_spec = timeSF_take_t_match_spec_t(1)
    client.take_t_table_add_with_write1(sess_hdl, dev_tgt, match_spec)

    for kv in r:
        match_spec = timeSF_getgoodsign_match_spec_t(kv[0])
        action_spec = timeSF_ggs_action_spec_t(kv[1])
        client.getgoodsign_table_add_with_ggs(sess_hdl, dev_tgt, match_spec,action_spec)



    match_spec1 = timeSF_rdmz_match_spec_t(0)
    match_spec2 = timeSF_rdmz_match_spec_t(1)
    match_spec3 = timeSF_rdmz_match_spec_t(2)
    match_spec4 = timeSF_rdmz_match_spec_t(3)
    client.rdmz_table_add_with_rm1(sess_hdl, dev_tgt, match_spec2)
    client.rdmz_table_add_with_rm1(sess_hdl, dev_tgt, match_spec4)
    client.rdmz_table_add_with_rm2(sess_hdl, dev_tgt, match_spec1)
    client.rdmz_table_add_with_rm2(sess_hdl, dev_tgt, match_spec3)
	
    match_spec1 = timeSF_restore_match_spec_t(0)
    match_spec2 = timeSF_restore_match_spec_t(1)
    match_spec3 = timeSF_restore_match_spec_t(2)
    match_spec4 = timeSF_restore_match_spec_t(3)
    client.restore_table_add_with_rs1(sess_hdl, dev_tgt, match_spec2)
    client.restore_table_add_with_rs1(sess_hdl, dev_tgt, match_spec4)
    client.restore_table_add_with_rs2(sess_hdl, dev_tgt, match_spec1)
    client.restore_table_add_with_rs2(sess_hdl, dev_tgt, match_spec3)



    print "Port add done."
    sign = 1
    match_spec = timeSF_sign_t_match_spec_t(0)
    action_spec = timeSF_signing_action_spec_t(sign)
    try:
        client.sign_t_table_delete_by_match_spec(sess_hdl, dev_tgt, match_spec)
    except:
        pass
    client.sign_t_table_add_with_signing(sess_hdl, dev_tgt, match_spec, action_spec)
    '''considersleep'''
    time1=datetime.datetime.now()
    x=0
    y=0
    z=0
    while True:
        time2=datetime.datetime.now()
        if (time2-time1).total_seconds()>=switchtime and x==0:
            print ((time2-time1).total_seconds())
            sign1=(sign+2)%4
            x=1
            match_spec = timeSF_sign_t_match_spec_t(0)
            action_spec = timeSF_signing_action_spec_t(sign1)
            try:
                client.sign_t_table_delete_by_match_spec(sess_hdl, dev_tgt, match_spec)
            except:
                pass
            client.sign_t_table_add_with_signing(sess_hdl, dev_tgt, match_spec, action_spec)
            time.sleep(0.1)
            client.register_reset_all_rm_a(sess_hdl, dev_tgt)
            if sign1==3:
                client.register_reset_all_fs_a_1_1(sess_hdl, dev_tgt)
                client.register_reset_all_fs_b_1_1(sess_hdl, dev_tgt)
                client.register_reset_all_fs_c_1_1(sess_hdl, dev_tgt)
                client.register_reset_all_fs_d_1_1(sess_hdl, dev_tgt)
                print("clean sketch1")
            if sign1==0:
                client.register_reset_all_fs_a_2_1(sess_hdl, dev_tgt)
                client.register_reset_all_fs_b_2_1(sess_hdl, dev_tgt)
                client.register_reset_all_fs_c_2_1(sess_hdl, dev_tgt)
                client.register_reset_all_fs_d_2_1(sess_hdl, dev_tgt)
                print("clean sketch2")
            time6=datetime.datetime.now()  
            print ((time6-time2).total_seconds())
        if (time2-time1).total_seconds()>=totaltime:
            print ((time2-time1).total_seconds())
            z=1
            time1=time2
            sign=3-sign
            x=0
            y=0
            match_spec = timeSF_sign_t_match_spec_t(0)
            action_spec = timeSF_signing_action_spec_t(sign)
            try:
                client.sign_t_table_delete_by_match_spec(sess_hdl, dev_tgt, match_spec)
            except:
                pass
            client.sign_t_table_add_with_signing(sess_hdl, dev_tgt, match_spec, action_spec) 
			

def main():
    connect()

if __name__ == "__main__":
    main()
