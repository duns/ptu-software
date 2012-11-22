#!/bin/sh

export GST_DEBUG=2

LOGFILENAME=/tmp/`date +"%y%m%d-%H%M%S"`.log

stop_execution()
{
	echo "********************  SESSION FINISH (`date`)  ********************" >> ${LOGFILENAME}
	unset GST_DEBUG

	exit 1	
}

control_c()
{
	echo "********************  SESSION FINISH (`date`)  ********************" >> ${LOGFILENAME}
	unset GST_DEBUG
	
	exit 0
}

echo "-------------------- SESSION START (`date`) --------------------" >> ${LOGFILENAME}

trap control_c SIGINT

while true
do
	echo `date +"%H:%M:%S.%N"` " - Raising Source" >> ${LOGFILENAME}
	videosource --config-path=/etc/videosource/vsource.conf 2>> ${LOGFILENAME} 
	
	if [ -f /tmp/vs.flag ];
	then
		stop_execution	
	fi
done
