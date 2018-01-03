#!/bin/sh
start='iwpriv apcli0 elian start'
stop='iwpriv apcli0 elian stop'
result='iwpriv apcli0 elian result'
clear='iwpriv apcli0 elian clear'
n=0
$start
sleep 1
t=$1
if [ "$1" = ""]
then
	t=60
fi
while [ "$($result | awk -F ',' '{print $2}' | awk -F '=' '{print $2}')" = "" -a $n -le $t ]
do
	$result
	n=$(($n+2))
	sleep 2
done
ssid="$($result | awk -F ',' '{print $2}' | awk -F '=' '{print $2}')"
sleep 1
key="$($result | awk -F ',' '{print $3}' | awk -F '=' '{print $2}')"
sleep 1
if [ "$ssid" != "" ]
then
	#setwifi $ssid $key
	uci set wireless.@wifi-iface[1].ApCliSsid=$ssid
	uci set wireless.@wifi-iface[1].ApCliPassWord=$key
	uci commit
	echo "ssid:$ssid key:$key" > /dev/console
	sleep 10
	$stop
	$clear
	nr
else
	$stop
	$clear
fi

