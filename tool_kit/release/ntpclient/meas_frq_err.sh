#!/bin/sh
killall ntpclient
killall ntpd
killall crond

ntpclient -f 0 -s -h $1
ntp_path=/home/boyuu/ntp/
ntpclient -i 30 -c 60 -h $1>${ntp_path}meas_frq_err.log
awk -f ${ntp_path}rate.awk < ${ntp_path}meas_frq_err.log >${ntp_path}frq.err
${ntp_path}start_ntpd.sh
