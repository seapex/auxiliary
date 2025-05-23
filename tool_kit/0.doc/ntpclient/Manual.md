
## Programer manual of ntpclient
### readme
refer to README for detail.
ntpclient home page: http://doolittle.icarus.com/ntpclient/

```bash    
Usage: ntpclient [options]
options:
 -c count     stop after count time measurements (default 0 means go forever)
 -d           print diagnostics (feature can be disabled at compile time)
 -g goodness  causes ntpclient to stop after getting a result more accurate
                   than goodness (microseconds, default 0 means go forever)
 -h hostname  (mandatory) NTP server, against which to measure system time
 -i interval  check time every interval seconds (default 600)
 -l           attempt to lock local clock to server using adjtimex(2)
 -p port      local NTP client UDP port (default 0 means "any available")
 -q min_delay minimum packet delay for transaction (default 800 microseconds)
 -r           replay analysis code based on stdin
 -s           simple clock set (implies -c 1)
 -t           trust network and server, no RFC-4330 recommended cross-checks
```         
> ntpclient always sends packets to the server's UDP port 123.

### howto
refer to HOWTO for detail
The goal of ntpclient is not only to set your computer's clock right once, but keep it there.
adjtimex can be used.
The options discussed here are:
 -f    frequency (integer kernel units)
 -o    time offset in microseconds
 -t    kernel tick (microseconds per jiffy)
 
First, set the time approximately right:
`ntpclient -s -h $NTPHOST`
You should see a single line printed like
```
day    second    elapsed    stall   skew      dispersion  freq
36765  4980.373  1341.0     39.7    956761.4  839.2       0
```
- `day` -- day since 1900
- `second` -- seconds since midnight
- `elapsed` -- elapsed time for NTP transaction (us)
- `stall` -- internal server delay (us)
- `skew` -- difference between local time and server time (us)
- `dispersion` -- reported by server. see RFC-1305 (us)
- `freq` -- local clock frequency adjustment (Linux only, ppm*65536)

In the example above, clock was a bit more than 0.95 seconds fast, compared to the clock on $NTPHOST.
Now check that the clock setting worked.
`  ntpclient -c 1 -h $NTPHOST`
```
day    second    elapsed    stall   skew      dispersion  freq
36765  4993.512  1345.0     40.9    3615.3    839.2       0
```
So now the time difference is only a few ms.
On to measure the frequency calibration for your system.
If you're in a hurry, it's OK to only spend 20 minutes on this step.
` ntpclient -i 60 -c 20 -h $NTPHOST >$(hostname).ntp.log &`
Otherwise, it is better to run for 24 hours on this step.
` ntpclient -i 300 -c 288 -h $NTPHOST >$(hostname).ntp.log &`
...

### custom
options added by the customer:
`-k skew_set`  set_time when skew(difference between local time and server time)>skew_set(ms).





