#!/bin/bash
inp=$1
if [[ $inp = "stop" ]]
then
        screen -X -S autocamWind quit;
	killall vlc;
	killall autocam;
        echo "stopped";
elif [[ $inp = "start" ]]
then
	screen -dmS autocamWind ./autocamAndVlc.sh;
echo "screen <<autocamWind>> started";
else
        echo 'Command format: ' $0 '[start|stop]'
fi
