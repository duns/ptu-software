#!/bin/sh
case "$1" in
	start)
		echo -n "Starting "
		ifup wlan0
		echo "."
		;;
	stop)
		echo -n "Stopping "
		echo "."
		;;
esac

