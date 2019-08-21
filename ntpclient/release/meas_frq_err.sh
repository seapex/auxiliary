#!/bin/sh
./ntpclient -f 0 -s -h $1
./ntpclient -i 30 -c 60 -h $1>$1.ntp.log -k 10000
awk -f rate.awk < $1.ntp.log >frq.err
