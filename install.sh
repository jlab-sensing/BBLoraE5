#!/bin/bash

function install_proj() {
	cd $1
	make
	make install
	cd ..
}

install_proj libipc
install_proj lora
install_proj pipestream
install_proj rocketlogger

install -m 644 scripts/logger.service /etc/systemd/system
install -m 777 scripts/logger.sh /usr/bin
