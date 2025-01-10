#!/bin/sh

if [ $# -gt 0 ] && [ $1 = "-r" ]; then
    killall ntpd
    killall crond
fi

adjtimex -f $(awk '/^new frequency/ {print $3}' /home/boyuu/ntp/frq.err)
ntpd
crond
