#!/bin/sh
#notes='E E D E P G Fs4  E E D E \
#set -x
note=A
sleepdur=0.25
[ -n "$1" ] && note = $1
[ -n "$2" ] && sleepdur = $2
PWMDEV=/dev/pwm8

	case $note in
	C)
		frequency=262;
		;;
	D)
		frequency=294;
		;;
	E)
		frequency=330;
		;;
	F)
		frequency=349;
		;;
	Fs)
		frequency=370;
		;;
	G)
		frequency=392;
		;;
	A)
		frequency=440;
		;;
	B)
		frequency=494;
		;;
	P)
		frequency=0;
		;;
	esac
   if [ $frequency -eq 0 ]; then
                echo 0 > $PWMDEV
        else
                echo "-"$frequency >  $PWMDEV
                echo 50 > $PWMDEV
        fi
	sleep $sleepdur


echo 0 > $PWMDEV
