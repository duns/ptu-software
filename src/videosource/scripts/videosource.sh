#!/bin/bash

SOURCEBIN=/home/root/workspace/ptu-software/bin/videosource
CFGFILE=/etc/videostream.conf/vsource_cfg

control_c()
{
	exit 0
}

trap control_c SIGINT

while true
do
	${SOURCEBIN} --config-path ${CFGFILE}
done

