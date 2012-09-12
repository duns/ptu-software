#!/bin/bash
DEST=192.168.3.204
PORT=5000
gst-launch -v v4l2src always-copy=FALSE ! video/x-raw-yuv,width=320,height=240,framerate=15/1 ! TIVidenc1 codecName=mp4enc engineName=codecServer ! rtpmp4vpay pt=96 ! udpsink host=${DEST} port=${PORT} sync=false
