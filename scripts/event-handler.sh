#!/bin/sh
PIPEFILE="/home/root/pipe_write"
PWMDEV="/dev/pwm8"
. /etc/profile

led_con_status()
{
        case $1 in
                0)
                        echo none > ${WPSS_LED_STAT}/trigger
                        echo none > ${WPSS_LED_BAT}/trigger
                ;;
                1)
                        echo timer > ${WPSS_LED_STAT}/trigger
                        echo none > ${WPSS_LED_BAT}/trigger
                ;;
                2)
                        echo default-on > ${WPSS_LED_STAT}/trigger
                        echo none > ${WPSS_LED_BAT}/trigger
                ;;
                3)
                        echo default-on > ${WPSS_LED_STAT}/trigger
                        echo default-on > ${WPSS_LED_BAT}/trigger
                ;;
                4)
                        echo none > ${WPSS_LED_STAT}/trigger
                        echo timer > ${WPSS_LED_BAT}/trigger
                ;;
                5)
                        echo none > ${WPSS_LED_STAT}/trigger
                        echo default-on > ${WPSS_LED_BAT}/trigger
                ;;
        esac
}
LASTBATTERY_LED_STATUS=3
led_con_status 3			
while true
do
	if [ -e "$PIPEFILE" ]; then
		read -t 2 line <> "$PIPEFILE"
		if [ -n "$line" ];then
			case $line in
				Event*_UpLevel_DoseRate)
				led_con_status 1			
				echo -1000 > ${PWMDEV}
				echo 50 > ${PWMDEV}
				;;	
				Event*_DownLevel_DoseRate)
				led_con_status ${LASTBATTERY_LED_STATUS}
				echo 0 > ${PWMDEV}
				;;	
				Event*_MidLevel_DoseRate)
				led_con_status ${LASTBATTERY_LED_STATUS}
				echo 0 > ${PWMDEV}
				;;	
#				Event2_UpLevel_DoseAccum)
#				led_con_status 1			
#				echo -1000 > ${PWMDEV}
#				echo 50 > ${PWMDEV}
#				;;	
#				Event2_DownLevel_DoseAccum)
#				led_con_status ${LASTBATTERY_LED_STATUS}
#				echo 0 > ${PWMDEV}
#				;;	
#				Event3_ValueChange_DoseAccum)
#				led_con_status ${LASTBATTERY_LED_STATUS}
#				echo 0 > ${PWMDEV}
#				;;	
				Event*_UpLevel_BatteryLevel)
				LASTBATTERY_LED_STATUS=5
				led_con_status $LASTBATTERY_LED_STATUS			
				;;	
				Event*_DownLevel_BatteryLevel)
				LASTBATTERY_LED_STATUS=1
				led_con_status $LASTBATTERY_LED_STATUS			
				;;	
				Event*_MidLevel_BatteryLevel)
				LASTBATTERY_LED_STATUS=3
				led_con_status $LASTBATTERY_LED_STATUS			
				;;	
			esac

		fi
	else
		led_con_status 2
		sleep 2
	fi

done
