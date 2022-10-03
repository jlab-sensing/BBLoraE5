#!/bin/bash

function install_proj() {
	cd $1
	make
	make install
	cd ..
}

# install applications
install_proj libipc
install_proj lora
install_proj pipestream
install_proj rocketlogger
install_proj csvstream

# install service
install -m 644 scripts/logger.service /etc/systemd/system
systemctl daemon-reload
# install top level executable
install -m 755 scripts/logger /usr/bin
