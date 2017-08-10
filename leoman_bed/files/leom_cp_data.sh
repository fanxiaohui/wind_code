#!/bin/sh

local gen_report=$1
local file=$2

[ "$file" == "" ] && return 0
[ ! -e $file ] && {
	/usr/bin/logger -t leoman_bed "[Shell]:File [$file] is NOT exist!"
	return 0
}

/bin/mount |grep -qi mmcblk || {
	/usr/bin/logger -t leoman_bed "[Shell]:SDcard has NOT be mounted"
	return 0
}

[ ! -d /mnt/mmcblk0p1/leoman ] && /bin/mkdir -p /mnt/mmcblk0p1/leoman
if [ $gen_report == 0 ]; then
	/bin/cp $file /mnt/mmcblk0p1/leoman/
else
	local reportfile=/tmp/leoman/"report-"${file##*/}
	/bin/cp $file /mnt/mmcblk0p1/leoman/
	/bin/beacon < $file > $reportfile
	rm -f $file
	[ $? != 0 ] && {
		/usr/bin/logger -t leoman_bed "[Shell]:beacon ret error !"
		/bin/rm -f $reportfile
		return 0
	}
	/usr/bin/logger -t leoman_bed "[Shell]:beacon report OK"
	/bin/mv $reportfile /mnt/mmcblk0p1/leoman/
	
	#if [ -e /mnt/mmcblk0p1/leoman/"report-"${file##*/} ]; then
		# ignore
	#fi
fi

