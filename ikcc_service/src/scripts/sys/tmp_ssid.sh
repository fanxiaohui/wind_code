#!/bin/sh
t=$1
if [ "$1" = "" ]
then
t=120
fi
ifconfig ra0 up
sleep $1
ifconfig ra0 down
