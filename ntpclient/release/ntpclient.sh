#!/bin/sh
# NTP Server Address
Addr=192.168.1.3
ntp_path=/home/boyuu/ntp/
if [ $# -gt 0 ];then
  FileVersion=1
  if [ $1 = "-v" ];then
    exit $FileVersion
  elif [ $1 = "-m" ];then
    ${ntp_path}meas_frq_err.sh $Addr &
    exit $FileVersion
  fi
fi

# Automatic synchronization cycle(unit:s)
SyncCycle=60
# Frequency error of crystal oscillator (unit:ppm*65536??)
FreqErr=0

killall ntpclient
#adjtimex -f $FreqErr
#ntpclient -s -i 15 -g 10000 -h $Addr
#ntpclient -i $SyncCycle -l -h $Addr >/tmp/ntpclient.log &
ntpclient -f $FreqErr -s -g 10000 -l -i $SyncCycle -h $Addr -t >/tmp/ntpclient.log &
