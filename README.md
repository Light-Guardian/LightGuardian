# LightGuardian

## Introduction
Network traffic measurement is central to successful networkoperations, especially for today’s hyper-scale networks.Although existing works have made great contributions, theyfail to achieve full-visibility, low overhead, and robustnesssimultaneously. We design LightGuardian to meet these threecriteria. Our key innovation is a (small) constant-sized datastructure, called sketchlet, which can be embedded in packetheaders. Specifically, we design a novel SuMax sketch to accuratelycapture flow-level information. SuMax can be dividedinto sketchlets, which are carried in-band by passing packetsto the end-hosts for aggregation, reconstruction, and analysis.We have fully implemented a LightGuardian prototype on atestbed with 10 programmable switches and 8 end-hosts ina FatTree topology, and conduct extensive experiments andevaluations. Experimental results show that LightGuardiancan obtain per-flow per-hop flow-level information within1.0~1.5 seconds with consistently low overhead, using only0.07% total bandwidth capacity of the network. We believeLightGuardian is the first system to collect per-flow per-hopinformation for all flows in the network with negligible overhead.

## About this Repo
- `cpp`: contains the codes implemented on CPU platform. We evaluate the performance of our SuMax sketch and LightGuardian's robustness on CPU platform. 
- `mininet` contains the codes implemented on Mininet. We evaluate LightGuardian’s performance in locating blackholes,loops, and abnormal jitters through Mininet case studies.
- `testbed` contains the codes related to our testbed. We have fully implemented our SuMax on a Tofino-40GbE switch, and implemented the three key functions on the endhosts (sending packets, receiving packets, reconstruction andperforming analysis).
- `utils`: miscellaneous functions.


