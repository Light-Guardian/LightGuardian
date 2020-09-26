## P4 implementation
The codes for flow size estimation. `flowSF.p4` is the data plane codes, `flowSF.py` is the control plane codes.

### Requirements
- Please compile and run the codes on a Tofino ASIC. 

### Usage
- To run the control plane, you must complie the p4 program `flowSF.p4` with compiling option `--with-thrift`
- Configure `ecmp.txt`, `range.txt` as you need.

