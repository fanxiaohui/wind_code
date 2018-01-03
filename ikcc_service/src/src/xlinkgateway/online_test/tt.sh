#!/bin/sh

port1=26638
port2=65510
macfile=./maclist
a=1
while read line
do
	
	echo "===$a get MAC: $line"
	cd ./t$a
	./ikcc_service $port1 $port2 $line &
	a=$(($a+1))
	port1=$(($port1+1))
	port2=$(($port2+1))
done < $macfile