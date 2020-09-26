## File Description

* ```analyzer_LG``` analyzes the packets received by each end-host after deploying *LightGuardian* on each switch.
* ```analyzer_INT``` analyzes the packets received by each end-host after deploying *INT* on each swtich.

## Usage

### Generate and Configure packet information

* Use ```tcpdump``` to capture packet information on each end-host.
* Collect all ```pcap``` files and configure ```pcap.config``` as follows, where *inet_addr* is the hexadecimal IP address of the end-host.

```
inet_addr_1 pcap_file_1
inet_addr_2 pcap_file_2
...
inet_addr_N pcap_file_N
```

### Analyze the ```pcap``` files

* ```analyzer_INT``` provides the total bandwidth usage of the network, and the bandwidth usage of *INT*.
* ```analyzer_LG``` provides the total bandwidth usage of the network, and the bandwidth usage of *LightGuardian*. It also provides how the reconstruction rate of each sketch changes over time.