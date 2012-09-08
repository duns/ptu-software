#!/bin/sh
WPSS_LED_STAT="/sys/class/leds/wpss:status"  #146
WPSS_LED_NET="/sys/class/leds/wpss:network" #144
WPSS_LED_BLUE="/sys/class/leds/overo:blue:COM"
WPSS_LED_BAT="/sys/class/leds/wpss:battery" #145
WPSS_LED_CON="/sys/class/leds/wpss:connection" #170

WPSS_SWITCH_3V=21
WPSS_SWITCH_5V=22

export WPSS_LED_STAT WPSS_LED_NET WPSS_LED_BLUE WPSS_LED_BAT WPSS_LED_CON WPSS_SWITCH_3V WPSS_SWITCH_5V
