#!/bin/sh
PIPEFILE="/home/root/pipe_write"
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
                        echo none > ${WPSS_LED_STAT}/trigger
                        echo timer > ${WPSS_LED_BAT}/trigger
                ;;
                4)
                        echo none > ${WPSS_LED_STAT}/trigger
                        echo default-on > ${WPSS_LED_BAT}/trigger
                ;;
        esac
}

led_con_status 3			
while true
do
	if [ -e "$PIPEFILE" ]; then
		read -t 2 line <> "$PIPEFILE"
		if [ -n "$line" ];then
			case $line in
				Event2_UpLevel_BatteryLevel)
				led_con_status 4			
				;;	
				Event2_DownLevel_BatteryLevel)
				led_con_status 1			
				;;	
				Event3_ValueChange_BatteryLevel)
				led_con_status 3			
				;;	
			esac
		fi
	else
		led_con_status 2
		sleep 2
	fi

done
