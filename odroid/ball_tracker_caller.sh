#!/bin/bash

while true; do
	out=$(~/sailcode/BoatVision/main 2>/dev/null) #> /tmp/ball_position 2>/dev/null
	if [[ $out != "N" ]]; then
		echo "$out" > /tmp/ball_position
		misses="0"
	fi
	#If the ball cannot be located 5 times in a row, update buffer to "ball not found" value
	misses=$((misses + 1))
	if [[ $misses -eq 5 ]]; then
		echo "$out" > /tmp/ball_position
		misses="0"
	fi
	echo "$misses"
done
