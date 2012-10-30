#!/bin/bash

SOURCEBIN=videosource
CFGFILE=/etc/videostream.conf/vsource_cfg

${SOURCEBIN} --config-path ${CFGFILE} &
VSPID=$!

control_c()
{
	while [ `ps --no-headers -p ${VSPID} -o "%p" | awk '{print $1}'` ]
	do
		kill ${VSPID} 2> /dev/null
	done
	exit 0
}

trap control_c SIGINT

while true
do
	if [ ! `ps --no-headers -p ${VSPID} -o "%p" | awk '{print $1}'` ] ; then
		${SOURCEBIN} --config-path ${CFGFILE} &
		VSPID=$!	
	fi
	
	sleep 5 
done

