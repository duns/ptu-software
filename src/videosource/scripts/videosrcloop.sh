#!/bin/sh

export GST_DEBUG=2

LOGFILENAME=/tmp/`date +"%y%m%d-%H%M%S"`.log
CAMERADEVICE=/dev/camera
SLEEPTIME=5

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
	if [ -e $CAMERADEVICE ];then
		echo `date +"%H:%M:%S.%N"` " - Raising Source" >> ${LOGFILENAME}
		videosource --config-path=/etc/videosource/vsource.conf 2>> ${LOGFILENAME} 
		echo `date +"%H:%M:%S.%N"` " - Source Exited" >> ${LOGFILENAME}
	else
		sleep $SLEEPTIME
		beep.sh B 0.1 >/dev/null 2>&1
	fi
done
