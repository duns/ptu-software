#!/bin/bash
STATE=0
LED1=22
LED2=21
LED3=147
LED=$LED1
[ -n "$1" ] && LED=$1
while true
do
	echo $STATE > /sys/class/gpio/gpio${LED}/value
	STATE=`echo "($STATE + 1)%2" |bc`
	sleep 1
done
	
