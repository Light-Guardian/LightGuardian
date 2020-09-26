## File Descriptions

- `client` contains the client's traffic generator and sketchlet aggregator. (end-host)
- `switch` contains the switch's control scripts and P4 codes.

## Usage
### Basic Setups
##### Mininet Setups
- Go to `./switch/p4src_*` folder
- Activate tmux and create duplicate windows
- With window A as the mininet console, type ` sudo p4run ` to evoke the Mininet. 
- After mininet_env established, we let window B enter `./switch/scripts` and we type `python routing-controller.py {f/i}` to set the tables in window B, where `f` represents the flowsize test, and `i` represents the interval test. 
- Now our Mininet has been fully established. And we are ready to perform the further tests.

##### Client Setups
- Go to corresponding `./client/client_*` folder.
- Type `make` to build traffic generator.
- Type `sudo ./traffic` to run clients, they will automatically generate flows, send&receive packets and save the sketchlets.
- Use tools in `/cpp/Simulation_Experiments` to analyze the collected log files in `./client/client_*/download_sketch` and `./switch/switch_log`

### More functions and tools
- Sending&Receiving Packets. 
	- In window C, type `mx h1` and `python simple_sender.py -h` to send packet from h1 and get helps.
	- In window D, type `mx h4` and `python simple_receiver.py {f/i}` to listen from an ethernet port.

- Swaping sketch using `switch/script/monitor.py`. 

- Generating errors using `switch/script/error.py` and `link up`/`link down` instructions in CLI.

- Generate switch logs. 
	- enter `switch/script/`, type `mx sw-cpu` and `python log_generator.py {f/i}` to generator switch-collected log.

- Setting core switch's priority.
	- Go to the directory where bmv2 has been installed. Go to `PATH\_TO\_BMV2/targets/simple\_switch/simple\_switch.h`. Look for the line `// #define SSWITCH_PRIORITY_QUEUEING_ON` and uncomment it.

	- Compile and install bmv2 again.

	- Copy `PATH\_TO\_P4C/p4include/v1model.p4` to another location. Add the following metadata fields into the `standard_metadata` struct. You will find a v1model.p4 in this directory.
	```
    @alias("queueing_metadata.qid")           bit<5>  qid;
    @alias("intrinsic_metadata.priority")     bit<3> priority;
	```
	- Copy the v1model.p4 to the global path: cp v1model.p4 /usr/local/share/p4c/p4include/. Note that every time you update p4c, this file will be overwritten and the metadatas will be removed.

	- Start Mininet.
	- Input in CLI: `p4switch_reboot sw_name --p4src ecmp_priority.p4`
	- The rest steps in Basic Setups. 

## Warnings for p4app.json Configuration
1. Change program item to the corresponding p4 file.
2. To use the logs, add `"cpu_port" : true` in every switch's configuration part, _e.g:_ `"s1": {"cpu_port" : true}`
3. Under different topology, we need change the `main` function of `switch/scripts/log_generator.py` and make sure the listening procedures' number is correpond to the real switch number.


## Other Hints 
Scripts in `\switch\scripts` contains the help information, use `-h` to get the detailed help. 