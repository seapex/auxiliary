@echo off
if {%1}=={} goto end1

start E:\Tools\Putty\putty.exe -ssh -P 12307 %1 -l root

:end1
@echo usage:	%0 ip