#!/bin/bash
#modprobe -r mt9v032
#modprobe mt9v032 auto_exp=0 #low_light=1
#modprobe mt9v032 hdr=0 low_light=1 
#modprobe mt9v032 hdr=0  auto_gain=0
date 012300002012
hwclock -w
/home/root/IO.sh
insmod /home/root/irqlat.ko
/home/root/ledblink.sh&
#modprobe mt9v032 hdr=0  vflip=1
insmod /home/root/pwm.ko timers=8,9,10 frequency=128
echo 100 > /dev/pwm9 
echo 100 > /dev/pwm10 

#ifconfig wlan0 |grep "inet addr" > /tmp/`hostname`
#date >> /tmp/`hostname`
#scp /tmp/`hostname` report@pcatlaswpss02:/tmp/`hostname` 2>&1 >/dev/null
#date > /tmp/date


linphonecsh init -c /home/root/.linphonerc
sleep 1
linphonecsh register --host pcatlaswpss02 --username 1002 --password pass2
sleep 1
linphonecsh generic 'autoanswer enable'
sleep 1
linphonecsh generic 'autoanswer enable'
/home/root/h264-server.sh &
#/home/root/watchbutton.sh &
exit


