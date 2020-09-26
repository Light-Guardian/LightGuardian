## Verification of Our SuMax Sketch

- We emulate and test the performance of our SuMax sketch using CAIDA2018 dataset.


### File Descriptions

- ``delay_distribution`` contains the codes to calculate WMRE of the delay_distribution of SuMax, CM and CU. 
	-  ``delay_chi2_wmre_vs_memory.cpp`` is able to emulate WMRE of Sumax, CM and CU with different memory. The ground truth obeys a Chi-Square distribution.
	- ``delay_union_wmre_vs_memory.cpp`` accomplishes the above task with uniform distributed ground truth. 
	- ``delay_mixed_wmre_vs_memory.cpp`` accomplishes the above task with mixed distributed ground truth.
	- ``delay_distribution_wmre_with_topk.cpp`` accomplishes the task of different top-k flows.

- ``flowsize`` contains the codes to calculate RE of flowsize, RE of cardinality, RE of entropy and WMRE of flowsize distribution. 
	- ``flowsize_ARE_vs_memory.cpp`` is able to calculate ARE of flowsize of SuMax, CM and CU with different memory.
	- ``card_dist_ent_vs_memory.cpp`` is able to calculate cardinality RE, flowsize distribution WMRE and entropy RE of SuMax, CM and CU with different memory.

- ``last_arrival-time`` contains the codes to calculate the correctness rate and ARE of last arrival-time of SuMax. ``last_arrival.cpp`` is the main file.

- ``max_inter-arrival`` contains the codes to calculate ARE of maximum inter-arrival time of SuMax. 
	- ``max_interval_vs_memory.cpp`` is able to calculate ARE of maximum inter-arrival time with different memory. 
	- ``max_interval_vs_topk.cpp`` is able to calculate ARE of maximum inter-arrival time with different top-k flows.

### How to Run
  - ``cd \task\path``
  
  - Change macro ``datapath`` as the path of your CAIDA2018 dataset. 
  
    ``#define datapath "\your\path"``
   
  - Run the ``.sh`` file in the folder. 
  
    ``/bin/sh *.sh``
