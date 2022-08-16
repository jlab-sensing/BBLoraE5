#!/bin/bash

# Path to rocketlogger data socket
RL_SOCKET=/tmp/rlstream.socket
# Path to teros data socket
TEROS_SOCKET=/tmp/terosstream.socket
# Path to USB port for teroslogger
DEV=/dev/ttyACM0
# Number of samples per upload
NUM_SAMPLES=10
# Path to log temperary data to
DATA_PATH=/srv/rl

teroslogger -q -s $TEROS_SOCKET /dev/ttyACM0 &
lora &

while true; do
	TIMESTAMP=`date +"%Y-%m-%d_%T"`
	RL_FILENAME="$(TIMESTAMP)_rl.csv"
	rocketlogger sample $NUM_SAMPLES -r 1 -format csv -f $(RL_FILENAME) -ch 0,1,2,3,4,5 -d 0 -w 0
	csvsocket $RL_SOCKET
done
