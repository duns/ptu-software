DEST=192.168.42.2
DEST=192.168.3.70
DEST=192.168.2.1
#DEST=192.168.3.204
PORT=5000
VIDEOSOURCE="videotestsrc ! video/x-raw-yuv,width=640,height=480,framerate=15/1" 
VIDEOSOURCE="v4l2src always-copy=FALSE ! video/x-raw-yuv,width=640,height=480,framerate=15/1" 
#VIDEOSOURCE="dc1394src ! video/x-raw-gray,width=640,height=480,framerate=15/1 !  ffmpegcolorspace ! video/x-raw-yuv" 
ENC="TIVidenc1 codecName=h264enc engineName=codecServer" 
#VIDEOSOURCE="videotestsrc  ! video/x-raw-yuv,width=640,height=480,framerate=15/1"

#gst-launch -v ${VIDEOSOURCE} ! autovideosink
echo gst-launch -v ${VIDEOSOURCE} ! ffmpegcolorspace ! ximagesink
#gst-launch -v ${VIDEOSOURCE} ! ffmpegcolorspace ! ximagesink
#gst-launch -v videotestsrc ! video/x-raw-yuv,width=640,height=480,framerate=15/1 ! TIVidenc1 codecName=h264enc engineName=codecServer ! rtph264pay name=pay0 pt=96 ! udpsink host=192.168.3.204 port=5000 sync=false
gst-launch v4l2src ! queue ! videorate ! video/x-raw-yuv,framerate=5/1 ! videoscale ! video/x-raw-yuv,width=640,height=320 ! ffmpegcolorspace ! ximagesink
