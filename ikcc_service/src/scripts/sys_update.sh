#!/bin/sh
wgetret=0
sysmd5sum=""
syssubversion=""
checkmd5=""
verfile="/sbin/system_version"
path="/tmp/"
sysname='openwrt-ramips-mt7628-wrtnode2p-squashfs-sysupgrade.bin'
sysvername='system_version'
syssubversion=$2
sysmd5sum=$1

md5temp=`md5sum $path$sysname`
checkmd5=`echo $md5temp|awk '{print $1}'`
if [ ! -f "$verfile" ]
then
	echo 1 > $verfile
fi
nativever=`cat /sbin/system_version`
if [ $sysmd5sum = $checkmd5 ]
then
	logger "check md5sum success!"
else
	logger "check md5sum fail! exit!"
	rm $path$sysname
	return 0
fi
if [ $nativever -eq $syssubversion ]
then
	logger "check firmware version not change! exit!"
	rm $path$sysname
	return 0
else
	logger "find a new firmware! will install now!" 
fi
#check firmware end
sysupgrade -v $path$sysname


