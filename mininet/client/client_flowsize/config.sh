#!/bin/bash

host_num=8

cat /dev/null > host.config

for host in $(seq 1 $host_num)
do
	echo h$host >> host.config
	ps au | grep mininet | grep -w h$host | awk '{print $2}' >> host.config
	mx h$host ifconfig | grep inet | grep -v 127.0.0.1 | awk '{print $2}' | tr -d "addr:" >> host.config
done
