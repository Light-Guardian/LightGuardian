#  description to files and folders
folder：
1. include:some p4 codes，including header and parser
   1. header.p4
   2. parser.p4
2. p4src_flowsize:codes for flowsize simulation
   1. ecmp.p4:p4 source code
   2. p4app.json:p4 configuration code
   3. log folder:mininet automatically generated
   4. pcap folder:mininet automatically generated
   5. ecmp.json:mininet automatically generated
   6. ecmp.p4i:mininet automatically generated
   7. topology.db:mininet automatically generated
3. p4src_interval:codes for interval simulation
4. scripts：python scripts
   1. error.py：generate loop，blackhole or reset the router table
   2. log_generator.py：generate switch's log
   3. monitor.py：swap the switch's sketch
   4. routing-controller.py：basic router configuration,register configuration,hash function configuration.
   5. simple_receiver.py
   6. simple_sender.py
   7. visor.py：print sketches of switch
5. switch_log



# usage
## basic steps 
1. get into the `../switch/p4src_……` folder
2. activate tmux and create duplicate windows
3. window A as the mininet console ,type ` sudo p4run ` to evoke the mininet
4. when the mininet_env is established, type `python routing-controller.py`  to set the tables in window B

## send packet & receive packet & listen to switch
5. in window C,type `mx h1` and type `python simple_sender.py -h` to send packet from h1 and get help
6. in window D,type `mx h4` and type `python simple_receiver.py` to listen from a eth port
7. in window B (after mini-step4),type `mx s7` ,and `python test.py -h` to minitor control plane of s7 


## question & problems 
1. why icmp will fail when s1 shutdown while udp will be ok: 1. ecmp 2. parser 3.the send back?
   SOLVED:by add a item in the hash of ecmp;
   BUT mininet `pingall` instruction may show that some hosts are still not connected yet, but using our sender/receiver,we can prove they are actually connected via tcp icmp and udp 
2. the packet's compatibility：solved


# core switch priority set
1. Go to the directory where you have bmv2 installed. Go to PATH_TO_BMV2/targets/simple_switch/simple_switch.h. look for the line // #define SSWITCH_PRIORITY_QUEUEING_ON and uncomment it.

2. Compile and install bmv2 again.

3. Copy and edit PATH_TO_P4C/p4include/v1model.p4 in another location. You will have to add the following metadata fields inside the standard_metadata struct. You can find a v1model.p4 with that added already in this directory.

    @alias("queueing_metadata.qid")           bit<5>  qid;
    @alias("intrinsic_metadata.priority")     bit<3> priority;

4. Copy the v1model.p4 to the global path: cp v1model.p4 /usr/local/share/p4c/p4include/. Remember that every time you update p4c this file will be overwritten and the metadata fields might be removed.

5. start mininet
6. input in CLI:`p4switch_reboot sw_name --p4src ecmp_priority.p4`
7. the rest steps
