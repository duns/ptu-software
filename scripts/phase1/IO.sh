#!/bin/bash
SYSDIR=/sys/class/gpio
LED1=21
LED2=22
LED3=147
BTN1=23
BTN2=14
BTN3=65
LEDLIGHTS1=144 #IR
LEDLIGHTS2=145 

enableIO()
{
	echo $1 \> $SYSDIR/export
	echo $1 > $SYSDIR/export
	echo $2 > $SYSDIR/gpio${1}/direction
	echo $2 \> $SYSDIR/gpio${1}/direction
	case "$2" in
		in)
			;;
		out)
	echo $3 \> $SYSDIR/gpio${1}/value
	echo $3 > $SYSDIR/gpio${1}/value
			;;
	esac
}
disableIO()
{
	echo $1 > $SYSDIR/unexport
}

setPins()
{
enableIO $BTN1 in
enableIO $BTN2 in
enableIO $BTN3 in
enableIO $LED1 out 0
enableIO $LED2 out 0
enableIO $LED3 out 0
enableIO $LEDLIGHTS1 out 1
enableIO $LEDLIGHTS2 out 1
#disableIO $BTN1 in
#disableIO $BTN2 in
}
setPins 
#setPins &>/dev/null &



