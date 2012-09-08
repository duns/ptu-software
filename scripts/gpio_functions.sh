#!/bin/sh
gpio_export()
{
	echo $1 > /sys/class/gpio/export
}
gpio_unexport()
{
	echo $1 > /sys/class/gpio/unexport
}
gpio_get()
{
	cat /sys/class/gpio/gpio${1}/value
}
gpio_set()
{
	echo $2 > /sys/class/gpio/gpio${1}/value
}
gpio_in()
{
	echo in > /sys/class/gpio/gpio${1}/direction
}
gpio_out()
{
	echo out > /sys/class/gpio/gpio${1}/direction
}


export  gpio_export gpio_unexport gpio_get gpio_set gpio_in gpio_out
