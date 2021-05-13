#!/bin/sh
# NTP Server Address
Addr=192.168.1.3

if [ $# -gt 0 ];then
  FileVersion=1
  if [ $1 = "-v" ];then
    exit $FileVersion
  elif [ $1 = "-m" ];then
    ./meas_frq_err.sh $Addr &
    exit $FileVersion
  fi
fi

# Automatic synchronization cycle(unit:s)
SyncCycle=60
# Frequency error of crystal oscillator (unit:ppm*65536??)
FreqErr=1208595

killall ntpclient
# ./ntpclient -f $FreqErr -s -h $NTPHOST
# ./ntpclient -l -i $SyncCycle -g 10000 -h $Addr >ntpclient.log
./ntpclient -f $FreqErr -s -l -i $SyncCycle -g 10000 -h $Addr >ntpclient.log
