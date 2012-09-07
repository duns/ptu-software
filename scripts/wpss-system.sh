#!/bin/sh
case "$1" in
	start)
		echo -n "Starting $DESC: $NAME"
		d_start
		echo "."
		;;
	stop)
		echo -n "Stopping $DESC: $NAME"
		d_stop
		echo "."
		;;
esac

