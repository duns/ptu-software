#!/bin/bash
#### commandserver script 
#### watches for button  presses and executes the appropriate command defined in the conffile
#### Author: Christos Papachristou (papachristou@novocaptis.com)
CONFFILE=$1
PIPENAME=/tmp/commandserver.fifo
PIPEFD=7
. $CONFFILE
KEYCODES="$PWRBTNKEYCODE $VOLUMEUPKEYCODE $VOLUMEDOWNKEYCODE $PANICKEYCODE $CALLCKEYCODE"
ECHO=echo
#ECHO=true
test_time_elapsed()
{
	RET=`echo $1-$2\>$3 | bc -l`
	[ $RET -eq 1 ] && return 0
	return 1
}

eventhandler()
{
	echo eventhandler start
	exec 7<>"${PIPENAME}"
	for CODE in $KEYCODES
		do

	BUTTONIGNORE[$CODE]=0
	BUTTONDOWN[$CODE]=-1
	BUTTONPRESSTIME[$CODE]=0
		done

	while true;
		do
#			READLINE=`echo "exec 7<> ${PIPENAME};read -u7 -t1 TIME CODE VAL ;RETVAL=\\$?;echo set TIME=\\$TIME \; CODE=\\$CODE \; VAL=\\$VAL ;return \\$RETVAL" |/bin/sh`
			READLINE=`echo "exec 7<> ${PIPENAME};read -u7 -t1 LINE;RETVAL=\\$?;echo \\$LINE;return \\$RETVAL" |/bin/sh`
			RETVAL=$?
			TIME=`echo $READLINE | awk '{print $1}'`
			CODE=`echo $READLINE | awk '{print $2}'`
			VAL=`echo $READLINE | awk '{print $3}'`
			case $RETVAL in
				0)
					[ $CODE = $PWRBTNKEYCODE ] && VAL=`echo ! $VAL | bc`
					case ${VAL} in
					0)
#						${ECHO} button down
						BUTTONPRESSTIME[$CODE]=$TIME
						BUTTONDOWN[$CODE]=1
						;;
					1)
#						${ECHO} button up
						BUTTONDOWN[$CODE]=0
						;;
					esac
#				${ECHO} event $TIME / $CODE / $VAL
				;;
				1)
				TIME=`date +%s.%N`
				;;
				*)
					echo $RETVAL $TIME  what
				;;

			esac
			for CODE in $KEYCODES
				do
					if [ ${BUTTONDOWN[$CODE]} == 1 ];then
						case $CODE in

							$PWRBTNKEYCODE)
								${ECHO} powerbutton pressed
								BUTTONDOWN[$CODE]=2
						       ;;
							$VOLUMEUPKEYCODE)
								${ECHO} volume up pressed
							;;
							$VOLUMEDOWNKEYCODE)
								${ECHO} volume down pressed
							;;
							$PANICKEYCODE)
								${ECHO} panic pressed
							;;
							$CALLCKEYCODE)
								${ECHO} call pressed
							;;
						esac
					fi
					if [ ${BUTTONDOWN[$CODE]} == 0 ];then
						if [ ${BUTTONIGNORE[$CODE]} -eq 0 ];then
							case $CODE in

								$PWRBTNKEYCODE)
									${ECHO} powerbutton released
									$PWRBTNSHORTPRESSCMD
						       		;;
								$VOLUMEUPKEYCODE)
									${ECHO} volume up released
								;;
								$VOLUMEDOWNKEYCODE)
									${ECHO} volume down released
								;;
								$PANICKEYCODE)
									${ECHO} panic released
								;;
									$CALLCKEYCODE)
									${ECHO} call released
								;;
							esac
						fi
						BUTTONIGNORE[$CODE]=0
						BUTTONDOWN[$CODE]=-1
					fi
					if [ ${BUTTONDOWN[$CODE]} == 2 ];then
						case $CODE in

							$PWRBTNKEYCODE)
								if  test_time_elapsed $TIME ${BUTTONPRESSTIME[$CODE]} $PWRBTONLONGPRESSSECS  ;then
									${ECHO} powerbuttonlongpress
									BUTTONDOWN[$CODE]=-1
									BUTTONIGNORE[$CODE]=1
									echo calling powerdown
									$PWRBTNLONGPRESSCMD
								fi
						       ;;
							$VOLUMEUPKEYCODE)
								echo volume up long press
							;;
							$VOLUMEDOWNKEYCODE)
								echo volume down  long press
							;;
							$PANICKEYCODE)
								echo panic long  press
							;;
							$CALLCKEYCODE)
								echo call long press
							;;
						esac
					fi
				done
		done
}
eventloop()
{
	DEVICE=$1
	echo eventloop $1
	stdbuf -i0 -o0 -e0 evtest $1 |  while read line;
#	evtest $1 |  while read line;
		do	
			EVENT=`echo "$line" | grep "Event: time "|grep -v " ----------"`
			[ -z "$EVENT" ] && continue
			TIME=`echo "$EVENT" | sed -e 's/^Event: time \([0-9]*\.[0-9]*\).*/\1/'`
			CODE=`echo "$EVENT" | sed -e 's/.*, code \([0-9]*\).*/\1/'`
			VAL=`echo "$EVENT" | sed -e 's/.*, value \([0-9]*\).*/\1/'`
			echo $TIME $CODE $VAL > ${PIPENAME}
#		COMMAND=$line
#			case $COMMAND in
#				'quit')
#					exit 0
#					;;
#				 *)
#					echo unknown command
#					;;
#			esac
		done
}

rm -f "${PIPENAME}"
mknod "${PIPENAME}" p

#watchdogloop&
#WATCHDOGLOOPPID=$!
eventhandler&
#EVENTHANDLERPID=$!
eventloop $GPIOBTN &
#EVENTLOOP0PID=$!
eventloop $PWRBTN &
#EVENTLOOP1PID=$!
trap "kill -9 -$$ " INT TERM EXIT
wait 
