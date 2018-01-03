#!/bin/sh

serverurl='http://192.168.8.127:8000/opkg/'
wgetret=0
installret=0
installed=0
ikccmd5sum=""
ikccsubversion=""
checkmd5=""
path="/tmp/"
verfile="/bin/ikcc/programinfo/version"
ikccname='ikcc_service_1_ramips_24kec.ipk'
ikccvername='ikcc_service_version'
ikccurl=$serverurl$ikccname
ikccverurl=$serverurl$ikccvername


binname=ikcc_service

wget -P $path $ikccurl && wget -P $path $ikccverurl && wgetret=1
#download
if [ $wgetret == 0 ]
then
        echo "wget fail"
	rm $path$ikccvername
	rm $path$ikccname
        return 0
else
	echo "wget:  "$ikccname"   success!"
fi
#download end

#check ikcc ipk variable 
for line in `cat $path$ikccvername`
do
	idd=`echo $line|awk -F '=' '{print $2}'`
	if [ `echo $line|awk -F '=' '{print $1}'` == "ikcc_md5" ]
	then
		ikccmd5sum=$idd
		echo "online ikcc_md5  : "$ikccmd5sum
	elif [ `echo $line|awk -F '=' '{print $1}'` == "ikcc_subversion" ]
	then
		echo "ikcc_subversion: "$idd
		ikccsubversion=$idd
	fi	
done
md5temp=`md5sum $path$ikccname`
checkmd5=`echo $md5temp|awk '{print $1}'`
echo "native checkmd5  : "$checkmd5

if [ ! -f "$verfile" ]
then
	echo 0 > $verfile
fi
nativever=`cat /bin/ikcc/programinfo/version`
echo "nativever: "$nativever
echo "***********************"
if [ $ikccmd5sum = $checkmd5 ]
then
	echo "check md5sum success!"
else
	echo "check md5sum fail! exit!"
	rm $path$ikccvername && rm $path$ikccname
	return 0
fi
if [ $nativever -eq $ikccsubversion ]
then
	echo "check ikcc version not change! exit!"
	rm $path$ikccvername
	rm $path$ikccname
	return 0
else
	echo "find a new ikcc ipk! will install" 
fi
#check ipk end

rm $path$ikccvername

#check pack has installed
opkg list |grep $binname && installed=1

if [ $installed == 0 ]
then
		opkg install $path$ikccname && installret=1
else
		pid=$(pidof ikcc_service)
		kill $pid
		opkg remove $binname
		opkg install $path$ikccname && installret=1
fi


if [ $installret == 1 ]
then
#	binid=`ps|grep $binname|grep -v grep|awk '{print $1}'`
#	kill $binid
        echo "install success!"
	mv $path$ikccname /bin/ikcc/programinfo/
	echo $ikccsubversion > $verfile
	echo "backup ipk ok"
#	reboot
else
	rm $path$ikccname
	echo "install fail! restore now!"
	opkg install /bin/ikcc/programinfo/$ikccname

fi


