#!/bin/bash

stty -F /dev/ttyACM0 cs8 57600 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts

if ! [[ -c /dev/ttyACM0 ]]; then
	echo "Arduino not found."
	exit 1
fi

checkRequest=$(printf "R\r\n")
echo $checkRequest

while true; do
	cat /dev/ttyACM0 | while read request; do
		echo $request
		if [[ $request = $checkRequest ]]; 
		then
			cat /tmp/ball_position > /dev/ttyACM0
			sleep 1
			echo -n "Request responded: "
			read request;
			echo $request
		else
			echo "No request received"
		fi
	#	sleep 1
	done
done
