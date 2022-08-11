#!/bin/bash

RL_SOCKET=/tmp/rlstream.socket
TEROS_SOCKET=/tmp/terosstream.socket
DEV=/dev/ttyACM0

rocketlogger meter -r 1 -ch 0,1,2,3,4,5 -d 0 -w 0 > pipesocket $RL_SOCKET &
teroslogger -q -s $TEROS_SOCKET /dev/ttyACM0 &
lora
