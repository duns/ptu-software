#!/bin/bash
#notes='E E D E P G Fs4  E E D E \
notes="C16 D16 E16 F16 G16 A16 B16 \
5C16 5D16 5E16 5F16 5G16 5A16 5B16 \
6C16 6D16 6E16 6F16 6G16 6A16 6B16 \
7C16 7D16 7E16 7F16 7G16 7A16 7B16 8C16" 
notes="B16 5Ds16 B16 5Fs16 B16 5Ds16 \
 5Ds16 B16 5Fs16 B16 \
 5Ds16 B16 5Fs16 B16 \
 5Ds16 B16 5Fs16 B16 \
 5Ds16 B16 5Fs16 B16 \
 5Ds16 B16 5Fs16 B16 \
 5Ds16 B16 5Fs16 B16 \
 5Ds16 B16 5Fs16 B16 \
 5E16 B16 5G16 B16 \
 5E16 B16 5G16 B16 \
 5E16 B16 5G16 B16 \
 5E16 B16 5G16 B16 \
 5E16 B16 5G16 B16 \
 5E16 B16 5G16 B16 \
 5E16 B16 5G16 B16 "
notes="C16 D16 E16 F16 G16 A16 B16 5C16"
BASEDUR=2
PWMDEV=/dev/pwm8
TUNESBIN=/usr/bin/tunes
[ -n "$1" ] && notes="$@"
echo $notes |  $TUNESBIN $PWMDEV $BASEDUR


