#!/bin/bash
build/./autocam rtsp://192.168.11.19:554\
 --cascadeProf=haarcascade_profileface.xml\
 --cascadeFront=haarcascade_frontalface_alt.xml\
 --recordResult=0\
 --scale=3\
 --scaleFactor=2\
 --face2shot=3\
 --zoomThr=0.3\
 --zoomSpeedMax=0\
 --zoomSpeedInc=0.0\
 --maxStepX=1.25\
 --maxStepY=1\
 --resultWidth=854\
 --resultHeight=480\
 --streamToStdOut=1\
 | cvlc fd://0\
 --demux=rawvideo\
 --rawvid-fps=25\
 --rawvid-width=854\
 --rawvid-height=480\
 --rawvid-chroma=RV24\
 --sout '#transcode{vcodec=mp4v,acodec=mpga,vb=2000,ab=32}:rtp{sdp=rtsp://:5025/dptz.sdp}'
