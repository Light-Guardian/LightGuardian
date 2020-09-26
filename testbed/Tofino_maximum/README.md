## P4 implementation
The codes for packet-interval estimation. `timeSF.p4` is the data plane codes, `timeSF.py` is the control plane codes.

### Requirements
- Please compile and run the codes on a Tofino ASIC. 

### Usage
- To run the control plane, you must complie the p4 program `timeSF.p4` with compiling option `--with-thrift`
- Configure `ecmp.txt` as you need.

