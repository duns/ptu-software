#!/bin/bash
LED1=144
LED2=145



#???
OUT1=21
BUZZER=147 #PWM8
BTN1=65
BTN2=14
BTN3=23
#17 12


echo $LED1 > /sys/class/gpio/export
echo $LED2 > /sys/class/gpio/export
echo will blink LED1
sleep 1
echo 1 > /sys/class/gpio/gpio${LED1}/value
sleep 1
echo 0 > /sys/class/gpio/gpio${LED1}/value
sleep 1
echo 1 > /sys/class/gpio/gpio${LED1}/value
echo will blink LED2
sleep 1
echo 1 > /sys/class/gpio/gpio${LED2}/value
sleep 1
echo 0 > /sys/class/gpio/gpio${LED2}/value
sleep 1
echo 1 > /sys/class/gpio/gpio${LED2}/value


