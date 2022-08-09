## Rocketlogger MFC Firmware

Holds the rocketlogger firmware to monitor Microbial Fuel Cells

### Building

To build and install the `teroslogger` utility. This will copy any relavent files to their intended place `/usr/bin/` for executables, and `/etc/systemd/system` for service files.
```
cd teroslogger
make
sudo make install
```

### Running

The intented way to run the logging utility is to use the provided systemd service files. It can be setup to run at startup and can track logs of each of the loggers.

```
# Enable the service on startup
systemctl enable logger@lora.service
# Start the service if not already running
systemctl start logger@lora.service
# Check the status of the service
systemctl status logger@lora.service
```
