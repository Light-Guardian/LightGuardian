## File Descriptions

- `include` contains the related P4 codes. 
   - `header.p4` defines the header used in P4. 
   - `parser.p4` defines the parser and deparser used in P4.
- `p4src_sum` contains the codes for the sum tasks simulation (flow size, flow size distribution, entropy, cardinality, and latency distribution). 
   - `ecmp.p4`: P4 source codes.
   - `p4app.json`: P4 configuration codes.
   - `ecmp_priority.p4`: P4 configuration codes that set the packet priority.
- `p4src_maximum` contains the codes for maximum tasks simulation (last arrival time, maximum inter-arrival time). Basically the same with `p4src_sum` but implements different function.
   - `ecmp.p4`: P4 source codes.
   - `p4app.json`: P4 configuration codes.
   - `ecmp_priority.p4`:P4 configuration codes that set the packet priority.
- `p4src_simple`: ECMP logic implementation.
   - `ecmp.p4`: P4 source codes.
   - `header.p4` defines the header used in P4. 
   - `parser.p4` defines the parser and deparser used in P4.
   - `routing_controller.py` automatically fills the route table. 
   - `iface_moniter.sh` and `iface_moniter.py` are used to monitor per-switch per-port packets.
- `scripts` contains python scripts. 
   - `error.py`: generate loops and blackholes OR reset the router table. 
   - `log_generator.py`: generate switch's log. 
   - `monitor.py`: swap the switch's sketch.  
   - `routing-controller.py`: basic configuration of router, register, and hash functions.
   - `simple_receiver.py`: a packet receiver for testing.
   - `simple_sender.py`: a packet sender for testing.
   - `visor.py`: print the sketches.




