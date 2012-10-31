#!/bin/sh
. /etc/profile
case "$1" in
	start)
		echo "Starting WPSS system services "
		echo "Bringing up wireless interface"
#		systemctl restart NetworkManager.service
#		ifup wlan0
		echo "enabling gpio ${WPSS_SWITCH_3V} (3.3V enable)"
		gpio_export ${WPSS_SWITCH_3V}
		gpio_out ${WPSS_SWITCH_3V}
		gpio_set ${WPSS_SWITCH_3V} 1
		
		echo "enabling gpio ${WPSS_SWITCH_5V} (5V enable)"
		gpio_export ${WPSS_SWITCH_5V}
		gpio_out ${WPSS_SWITCH_5V}
		gpio_set ${WPSS_SWITCH_5V} 1
		echo "enabling gpio12 as input "
		devmem2 0x480025D8 h 0x10c


		echo heartbeat > ${WPSS_LED_STAT}/trigger
		echo timer > ${WPSS_LED_NET}/trigger
#		echo heartbeat > ${WPSS_LED_BLUE}/trigger
		echo none > ${WPSS_LED_BAT}/trigger
		echo none > ${WPSS_LED_CON}/trigger
		echo "."

		;;
	stop)
		echo -n "Stopping "
		echo "."
		echo "disabling gpio ${WPSS_SWITCH_3V} (3.3V enable)"
		gpio_out ${WPSS_SWITCH_3V}
		gpio_set ${WPSS_SWITCH_3V} 0
		gpio_unexport ${WPSS_SWITCH_3V}
		
		echo "disabling gpio ${WPSS_SWITCH_5V} (5V enable)"
		gpio_out ${WPSS_SWITCH_5V}
		gpio_set ${WPSS_SWITCH_5V} 0
		gpio_unexport ${WPSS_SWITCH_5V}

		echo default-on > ${WPSS_LED_NET}/trigger
#		echo heartbeat > ${WPSS_LED_BLUE}/trigger
		echo default-on > ${WPSS_LED_BAT}/trigger
		echo default-on > ${WPSS_LED_CON}/trigger

		;;
esac

