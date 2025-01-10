#!/bin/sh
adjtimex -f $(awk '/^new frequency/ {print $3}' /home/boyuu/ntp/frq.err)
