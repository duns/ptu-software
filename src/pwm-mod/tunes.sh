#!/bin/bash
#notes='E E D E P G Fs4  E E D E \
notes="C16 D16 E16 F16 G16 A16 B16 \
5C16 5D16 5E16 5F16 5G16 5A16 5B16 \
6C16 6D16 6E16 6F16 6G16 6A16 6B16 \
7C16 7D16 7E16 7F16 7G16 7A16 7B16 8C16" 
notes="C16 D16 E16 F16 G16 A16 B16 5C16"
basedur=2.0
PWMDEV=/dev/pwm8

	numnotes=0
for note in $notes
do
	register=`echo $note | sed -e 's/[a-zA-Z]*[0-9]*$//'`
	duration=`echo $note | sed -e 's/[0-9]*[a-zA-Z]*//'`
	note=`echo $note | sed -e 's/[0-9]*//g'`
	[ -z $register ] && register=4 
	[ -z $duration ] && duration=8
#echo $note $register $duration
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
	sleepdur=`echo "scale=2;$basedur/$duration"| bc  `
	frequency=`echo $frequency \* 2 ^ \( $register - 4 \) | bc `
#echo $note $frequency $duration
			
#	echo $sleepdur $frequency
	sdur[$numnotes]=$sleepdur
	freq[$numnotes]=$frequency
	(( numnotes++ ))

done

for  (( i=0 ; i<numnotes ; i++ )) 
do
	frequency=${freq[$i]}
	sleepdur=${sdur[$i]}
   if [ $frequency -eq 0 ]; then
                echo 0 > $PWMDEV
        else
                echo "-"$frequency >  $PWMDEV
                echo 50 > $PWMDEV
        fi
	sleep $sleepdur

done
echo 0 > $PWMDEV
