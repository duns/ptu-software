#!/bin/bash
FREQ=1000
DUTY=50
PIN=144
[ -n "$1" ] && PIN=$1
[ -n "$2" ] && FREQ=$2
[ -n "$3" ] && DUTY=$3

case $PIN in
145)
BASEADDR=48086000
PADCONFADDR=48002176
BASEFREQ=32000
;;
144)
echo 144
BASEADDR=49040000
PADCONFADDR=48002174
BASEFREQ=13000000
;;
esac



DUTY=`echo "100-$DUTY" | bc `
DUTYHEX=`echo "obase=16;ibase=10;$DUTY" | bc `
DIV=`echo "obase=16;ibase=10;$BASEFREQ / $FREQ" | bc `
REG=`echo "obase=16;ibase=16;FFFFFFFF+1-$DIV" | bc`
NUMSETTINGS=`echo "obase=16;ibase=16;FFFFFFFE-$REG"|bc`
echo 2222
echo "obase=16;ibase=16;$REG + $NUMSETTINGS * $DUTY"
TMARVAL=`echo "obase=16;ibase=16;$REG + $NUMSETTINGS * $DUTYHEX / 64"|bc`

echo $DIV $REG $NUMSETTINGS $TMARVAL
#exit

TMAR=`echo "obase=16;ibase=16;$BASEADDR + 38"|bc`
TLDR=`echo "obase=16;ibase=16;$BASEADDR + 2C"|bc`
TCRR=`echo "obase=16;ibase=16;$BASEADDR + 28"|bc`
TCLR=`echo "obase=16;ibase=16;$BASEADDR + 24"|bc`
#echo 1 > /sys/class/gpio/gpio${PIN}/value
case $DUTY in
0)
devmem2 0x${PADCONFADDR} h 0x0004
echo 0 > /sys/class/gpio/gpio${PIN}/value
	exit
	;;
100)
devmem2 0x${PADCONFADDR} h 0x0004
echo 1 > /sys/class/gpio/gpio${PIN}/value
	exit
	;;
esac
#devmem2 0x${TCLR} w 0
devmem2 0x${TLDR} w 0x$REG
devmem2 0x${TMAR} w 0x$TMARVAL
devmem2 0x${TCRR} w 0x$REG
devmem2 0x${TCLR} w 0x01843
devmem2 0x${PADCONFADDR} h 0x0002

