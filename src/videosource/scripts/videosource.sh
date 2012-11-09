#!/bin/bash

SOURCEBIN=videosource
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

