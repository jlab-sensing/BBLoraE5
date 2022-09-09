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
install_proj csvstream

install -m 644 scripts/logger.service /etc/systemd/system
install -m 755 scripts/logger /usr/bin
