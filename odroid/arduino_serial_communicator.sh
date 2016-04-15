#!/bin/bash

if ! [[ -c /dev/ttyACM0 ]]; then
	echo "Arduino not found."
	exit 1
fi

while true; do
	cat /dev/ttyACM0 | while read request; do
		echo $request
		if [[ $request="Request" ]]; then
			cat /tmp/ball_position > /dev/ttyACM0
	#		sleep 1
			echo -n "Request responded: "
			read request;
			echo $request
		else
			echo "No request received"
		fi
	#	sleep 1
	done
done
