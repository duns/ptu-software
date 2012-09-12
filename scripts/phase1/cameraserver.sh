DEST=10.0.1.17
DEST=pcatlaswpss02-vpn-tcp
DEST=192.168.3.204
DEST=192.168.3.70
PORT=5000
PORT2=5002
WIDTH=480 ; HEIGHT=360 # e-con
WIDTH=360 ; HEIGHT=360 # e-con
WIDTH=416 ; HEIGHT=376 # e-con
WIDTH=320 ;  HEIGHT=240 # caspa
WIDTH=664 ;  HEIGHT=524 # firefly
WIDTH=640 ;  HEIGHT=480 # firefly
WIDTH=720 ;  HEIGHT=564 # e-con
FRAMERATE=30
FRAMERATE=10
FRAMERATEDIV=1
SUDO=""
CAMNO=0
set -x

ENCODING=MJPEG
#ENCODING=MJPEGHARD
ENCODING=H264
#ENCODING=H264SOFT
CAM=test
CAM=e-con
#CAM=firefly
VMODE=69
MJPEGBITRATE=5000000
MJPEGQUALITY=90
H264BITRATE=6000000
NETSINK="udpsink host=${DEST} port=${PORT} sync=false"
#NETSINK="tcpserversink host=localhost port=${PORT} sync=false " 

[ -n "$1" ] && CAM="$1"
[ -n "$2" ] &&	WIDTH="$2"
[ -n "$3" ] && HEIGHT="$3"
[ -n "$4" ] && ENCODING="$4"
[ -n "$5" ] && FRAMERATE="$5"

case $CAM in
	stingray)
	CAMSETTINGS="camera-number=${CAMNO} whitebal_rv=-1 exposure=-1"
	CAMSETTINGS="camera-number=${CAMNO} whitebal_rv=500 whitebal_bu=500  exposure=1"
	CAMSETTINGS="camera-number=${CAMNO} whitebal_rv=-2 whitebal_bu=-2 exposure=-1"
	CAMSETTINGS="camera-number=${CAMNO} whitebal_rv=-1 whitebal_bu=-1 exposure=-1"
	CAMSETTINGS="camera-number=${CAMNO} whitebal_rv=230 whitebal_bu=500  exposure=1"
	CAMSETTINGS="camera-number=${CAMNO} whitebal_rv=-1 whitebal_bu=-1 exposure=-1 shutter=800 gain=-1"
	CAMSETTINGS="camera-number=${CAMNO} whitebal_rv=187 whitebal_bu=417 exposure=-1 shutter=800 gain=-1"
	CAMSETTINGS="camera-number=${CAMNO} whitebal_rv=-1 whitebal_bu=-1 exposure=-1 shutter=500 gain=-1"
	VSRC=dc1394src
	VMODE=67
	VIDEOSOURCE="dc1394src ${CAMSETTINGS} ! video/x-raw-yuv,width=${WIDTH},height=${HEIGHT},vmode=${VMODE},framerate=${FRAMERATE}/${FRAMERATEDIV} ! ffmpegcolorspace ! videorate "
	VMODE=88
	VIDEOSOURCE="dc1394src ${CAMSETTINGS} ! video/x-raw-bayer,width=${WIDTH},height=${HEIGHT},vmode=${VMODE},framerate=${FRAMERATE}/${FRAMERATEDIV},format=bggr,bpp=8 ! queue ! bayer2rgb  ! ffmpegcolorspace ! videorate "
	VIDEOSOURCE="dc1394src ${CAMSETTINGS} ! video/x-raw-gray,width=${WIDTH},height=${HEIGHT},vmode=${VMODE},framerate=${FRAMERATE}/${FRAMERATEDIV},bpp=8 ! queue ! ffmpegcolorspace ! videorate "
#	VIDEOSOURCE="videotestsrc ! video/x-raw-bayer,width=${WIDTH},height=${HEIGHT},vmode=${VMODE},framerate=${FRAMERATE}/1,rate=60,format=rggb ! queue ! bayer2rgb ! ffmpegcolorspace "
	#VIDEOSOURCE="dc1394src ! queue ! videorate ! ffmpegcolorspace ! video/x-raw-yuv,width=${WIDTH},height=${HEIGHT},framerate=${FRAMERATE}/1"
	;;
	firefly)
	CAMSETTINGS="pgr-cam=1 camera-number=${CAMNO} whitebal_rv=500 whitebal-bu=500 exposure=1000 shutter=500 gamma=1024"
	CAMSETTINGS="pgr-cam=1 camera-number=${CAMNO} whitebal_rv=500 whitebal-bu=500 exposure=-1 shutter=-1 gamma=1024"
	VSRC=dc1394src
	VIDEOSOURCE="dc1394src ! video/x-raw-bayer,width=${WIDTH},height=${HEIGHT},vmode=${VMODE},framerate=${FRAMERATE}/${FRAMERATEDIV},format=rggb,rate=${FRAMERATE} ! queue !   bayer2rgb ! ffmpegcolorspace ! videorate "
	VIDEOSOURCE="dc1394src ! video/x-raw-bayer,width=${WIDTH},height=${HEIGHT},vmode=${VMODE},framerate=${FRAMERATE}/${FRAMERATEDIV},format=rggb ! queue !   bayer2rgb ! ffmpegcolorspace ! videorate "
	VIDEOSOURCE="dc1394src ${CAMSETTINGS} ! video/x-raw-bayer,width=${WIDTH},height=${HEIGHT},vmode=${VMODE},framerate=${FRAMERATE}/${FRAMERATEDIV},format=rggb !   bayer2rgb ! ffmpegcolorspace "
#	VIDEOSOURCE="videotestsrc ! video/x-raw-bayer,width=${WIDTH},height=${HEIGHT},vmode=${VMODE},framerate=${FRAMERATE}/1,rate=60,format=rggb ! queue ! bayer2rgb ! ffmpegcolorspace "
	#VIDEOSOURCE="dc1394src ! queue ! videorate ! ffmpegcolorspace ! video/x-raw-yuv,width=${WIDTH},height=${HEIGHT},framerate=${FRAMERATE}/1"
	;;
	webcamjpeg)
	VSRC="v4l2src always-copy=FALSE"
VIDEOSOURCE="v4l2src device=/dev/video0  ! image/jpeg,width=${WIDTH},height=${HEIGHT},framerate=${FRAMERATE}/${FRAMERATEDIV}"   #!   tee name=displaysink ! queue "
DISPLAYSINK=""
;;
	logitech)
	VSRC="v4l2src always-copy=FALSE"
VIDEOSOURCE="v4l2src always-copy=false device=/dev/video0  ! video/x-raw-yuv,width=${WIDTH},height=${HEIGHT},framerate=${FRAMERATE}/${FRAMERATEDIV},format=(fourcc)I420,interlaced=false ! ffmpegcolorspace "   #!   tee name=displaysink ! queue "
DISPLAYSINK=""
;;
	webcam)
	VSRC="v4l2src always-copy=FALSE"
VIDEOSOURCE="v4l2src always-copy=false ! video/x-raw-yuv,width=${WIDTH},height=${HEIGHT},framerate=${FRAMERATE}/1,format=(fourcc)UYVY,interlaced=true  ! ffmpegcolorspace ! tee name=displaysink "
VIDEOSOURCE="v4l2src always-copy=false ! video/x-raw-yuv,width=${WIDTH},height=${HEIGHT},framerate=${FRAMERATE}/${FRAMERATEDIV},format=(fourcc)UYVY,interlaced=true  " #!   tee name=displaysink ! queue "
#VIDEOSOURCE="v4l2src device=/dev/video1 !  video/x-raw-yuv,width=${WIDTH},height=${HEIGHT},framerate=${FRAMERATE}/${FRAMERATEDIV},format=(fourcc)UYVY  " #!   tee name=displaysink ! queue "
VIDEOSOURCE="v4l2src always-copy=false device=/dev/video0  ! video/x-raw-yuv,width=${WIDTH},height=${HEIGHT},framerate=${FRAMERATE}/${FRAMERATEDIV},format=(fourcc)YUY2,interlaced=false ! ffmpegcolorspace "   #!   tee name=displaysink ! queue "
DISPLAYSINK=""
;;
	e-con)
	VSRC="v4l2src always-copy=FALSE"
#VIDEOSOURCE="v4l2src always-copy=FALSE !  video/x-raw-yuv,width=${WIDTH},height=${HEIGHT},framerate=${FRAMERATE}/1,format=(fourcc)I420 ! ffmpegcolorspace" #  e-con MJPEG
#VIDEOSOURCE="v4l2src always-copy=FALSE !  video/x-raw-yuv,width=${WIDTH},height=${HEIGHT},framerate=${FRAMERATE}/1,format=(fourcc)I420 ! ffmpegcolorspace" #  e-con MJPEG
VIDEOSOURCE="v4l2src ! queue ! videorate ! video/x-raw-yuv,framerate=${FRAMERATE}/1 ! videoscale ! video/x-raw-yuv,width=${WIDTH},height=${HEIGHT}! ffmpegcolorspace "
VIDEOSOURCE="v4l2src always-copy=false ! video/x-raw-yuv,width=${WIDTH},height=${HEIGHT},framerate=${FRAMERATE}/1,format=(fourcc)UYVY,interlaced=true  ! ffmpegcolorspace ! tee name=displaysink "
VIDEOSOURCE="v4l2src always-copy=false ! video/x-raw-yuv,width=${WIDTH},height=${HEIGHT},framerate=${FRAMERATE}/${FRAMERATEDIV},format=(fourcc)UYVY,interlaced=true  " #!   tee name=displaysink ! queue "
DISPLAYSINK=" displaysink. ! queue !  videoscale !  vide/x-raw-yuv,width=360,height=282  ! ffmpegcolorspace ! fbdevsink"
DISPLAYSINK=" displaysink. !  queue !  ffmpegcolorspace ! fakesink sync=false"
#DISPLAYSINK=" displaysink. !  queue !  ffmpegcolorspace !  fakesink "
#DISPLAYSINK=" displaysink. !  queue ! udpsink host=127.0.0.1 port=10000 "
#DISPLAYSINK=" displaysink. !  queue ! ffmpegcolorspace ! ximagesink display=:0"
#DISPLAYSINK=" displaysink. !  queue !  video/x-raw-yuv,width=${WIDTH},height=${HEIGHT},framerate=${FRAMERATE}/1,format=(fourcc)UYVY !  ffmpegcolorspace ! ximagesink display=:0"
DISPLAYSINK=""
;;
caspa)
	VSRC="v4l2src always-copy=FALSE"
VIDEOSOURCE="v4l2src always-copy=FALSE ! video/x-raw-yuv,width=${WIDTH},height=${HEIGHT},framerate=${FRAMERATE}/1 " #  caspa
;;
test)
	VSRC="videotestsrc"
VIDEOSOURCE="videotestsrc ! video/x-raw-yuv,width=${WIDTH},height=${HEIGHT},framerate=${FRAMERATE}/1,format=(fourcc)UYVY ! ffmpegcolorspace " 
;;
esac


case ${ENCODING} in
LOCALVIEW)
export DISPLAY=:0
#gst-launch -v ${VIDEOSOURCE}  ! video/x-raw-yuv,width=${WIDTH},height=${HEIGHT} ! ffmpegcolorspace ! videoscale !   ximagesink
#sudo gst-launch -v ${VIDEOSOURCE} ! ffmpegcolorspace ! videoscale ! video/x-raw-yuv,width=320,height=240 ! ffmpegcolorspace !  ximagesink
#sudo gst-launch -v ${VIDEOSOURCE} !   ffmpegcolorspace ! videoscale ! ffmpegcolorspace !  ximagesink
${SUDO} gst-launch -v ${VSRC} !  videoscale ! video/x-raw-yuv,width=320,height=240,framerate=${FRAMERATE}/${FRAMERATEDIV} ! ffmpegcolorspace !  ximagesink
exit 0
;;
H264)
ENC="TIVidenc1 codecName=h264enc engineName=codecServer iColorSpace=UYVY bitRate=${H264BITRATE} encodingPreset=2 rateControlPreset=2 "  #numInputBufs=3 " 
ENC="TIVidenc1 codecName=h264enc engineName=codecServer iColorSpace=UYVY bitRate=${H264BITRATE} encodingPreset=2 rateControlPreset=2 "  #numInputBufs=3 " 
PAY="rtph264pay name=pay0 pt=96 "
;;
H264SOFT)
ENC="x264enc " 
PAY="rtph264pay name=pay0 pt=96 "
;;
MJPEG)
#v4l2src always-copy=FALSE input-src=Composite \
ENC="TIImgenc1 engineName=codecServer resolution="\'${WIDTH}x${HEIGHT}\'" iColorSpace=UYVY oColorSpace=YUV420P qValue=${MJPEGQUALITY} " #! multipartmux "
#ENC="TIImgenc1 engineName=codecServer resolution='640x480' iColorSpace=UYVY oColorSpace=YUV420P qValue=75 " #! multipartmux "

#ENC="ffmpegcolorspace ! ffenc_mjpeg bitrate=${MJPEGBITRATE} me-method=zero" 
#ENC="ffenc_mjpeg "
PAY="rtpjpegpay pt=96"
;;
MJPEGSOFT)
ENC="ffmpegcolorspace ! ffenc_mjpeg bitrate=${MJPEGBITRATE} me-method=zero" 
PAY="rtpjpegpay pt=96"
;;
MJPEGTHROUGH)
#v4l2src always-copy=FALSE input-src=Composite \
ENC="queue " #! multipartmux "
PAY="rtpjpegpay pt=96"
;;
esac
#VIDEOSOURCE=videotestsrc

launchstream()
{
#gst-launch -v ${VIDEOSOURCE} ! TIVidenc1 codecName=h264enc engineName=codecServer ! rtph264pay name=pay0 pt=96 ! udpsink host=${DEST} port=${PORT} sync=false
#gst-launch -v ${VIDEOSOURCE} ! TIVidenc1 codecName=h264enc engineName=codecServer ! rtph264pay name=pay0 pt=96 ! multiudpsink clients=${DEST}:${PORT},${DEST}:${PORT2} sync=false &
echo testing > /tmp/launchlog
echo gst-launch -v ${VIDEOSOURCE} !  ${ENC} ! ${PAY} !  udpsink host=${DEST} port=${PORT} sync=false ${DISPLAYSINK} >> /tmp/launchlog
${SUDO} gst-launch -v ${VIDEOSOURCE} !  ${ENC} ! ${PAY} !  ${NETSINK} ${DISPLAYSINK} &
#sudo gst-launch -v ${VIDEOSOURCE} !  ${ENC} ! ${PAY} !  multiudpsink clients=${DEST}:${PORT},127.0.0.1:${PORT} ${DISPLAYSINK} &
#sudo gst-launch -v ${VIDEOSOURCE} !  ${ENC} ! ${PAY} !  fakesink
export PID=$!
}
launchstream 
[ -z ${DETACH} ] && wait
exit

launchstream > 2>&1 /tmp/camcaps 
sleep 2
CAPS=`sed /tmp/camcaps -n -e '/sprop/{s/.*sprop-parameter-sets=(string)\(.*\), payload.*/\1/
p
q
}'`
kill $PID
echo $CAPS > /tmp/camcaps
launchstream 
wait $PID
