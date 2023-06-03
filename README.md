Overview
========

This repo contains the firmware used to measure and upload MFC data to our visualization portal, [DirtViz](https://dirtviz.jlab.ucsc.edu). The measurement itself is done with a [RocketLogger](https://www.rocketlogger.ethz.ch/), and either sent to our database via ethernet or [LoRaWAN](https://lora-alliance.org/about-lorawan/). 

[Link to the Dirtviz Repo](https://github.com/jlab-sensing/DirtViz)

> This uploader only works on firmware versions greater than v2.0.0, due to the dependency on python.


Some Notes
----------
- The upload interval is implemented by sleeping at the end of the forever loop, therefore the upload interval is not strictly monotonic.
- The data has the scale applied after measurement and is stored in the standard units V/I, so the uploaded data *may* have rounding errors when measuring very small voltages and currents.

Installation
------------

### Installation via ansible

Ensure that you are able to SSH into each of the RocketLoggeers before trying to install via ansible. It is recommended that you put connection parameters in `~/.ssh/config` to simplify ssh connection parameters. Example as follows:

```
Host rocket*-jlab.ucsc.edu
	User rocketlogger
	Port 2322
	PubkeyAcceptedKeyTypes +ssh-rsa
	IdentityFile ~/.ssh/rocketlogger.default_rsa
```

Run the ansible install playbook. More information on creating `inventory.yaml` can be found on ansible tutorial page [Building an inventory](https://docs.ansible.com/ansible/latest/getting_started/get_started_inventory.html).

```
ansible-playbook -i ~/inventory.yaml install.yaml -K
```

### Manual Installation

> NOTE: The following is untested and some commands will require `sudo` access.

1. Install prerequisites
```
sudo apt-get update && sudo apt-get upgrade
sudo apt-get install python3 python3-pip python3-dev python3-setuptools python3-numpy python3-requests fake-hwclock
```
2. Setup `fake-hwclock.service`
```
sudo systemctl enable fake-hwclock
sudo systemctl start fake-hwclock
```
3. Clone repository
```
git clone https://github.com/jlab-sensing/Rocketlogger-Firmware.git
cd RocketLogger-Firmware
```
4. Install package (it may take ~20mins if numpy module needs to be built)
```
sudo python3 -m pip install -U pip
sudo pip3 install .
```
5. Copy config
```
sudo cp uploader.yaml /etc
```
6. Create storage directory
```
sudo mkdir -p /media/sdcard/uploader
sudo chmod 777 /media/sdcard/uploader
```
6. Install systemd service and enable on startup
```
cd uploader.service /etc/systemd/system
sudo systemctl daemon-reload
sudo systemctl enable uploader
# Start immediately
sudo systemctl start uploader
```


Configuring the Rocketloggers
-----------------------------

> NOTE: Only required if using LoRa module

Because of how heavily multiplexed the Rocketlogger pins are, the only UART bus that we can use is UART5. By default, the beaglebones don't come with UART5 enabled which means it has to be set up manually. Type `ls -l /dev/ttyO*`, and it should only list one line:
```
crw-rw---- 1 root tty  247, date time /dev/ttyO0
```

To enable UART5, type `sudo nano /boot/uEnv.txt`. It'll display a text file that should start with a link to some documentation and the name of the rocketlogger -- from there, add a line at the bottom saying
```
cape_enable=capemgr.enable_partno=BB-UART5
```

If you list the ttyO* devices again again, it should have added another line which signals that ttyO5 has been enabled.

From there, you can connect the LoRa module to the Beaglebone's UART5 pins (see [datasheet](https://docs.beagleboard.org/latest/boards/beaglebone/black/ch07.html)).


LoRaWAN
-------

Specifics on LoRaWAN and how it works can be found [here](https://lora-alliance.org/about-lorawan/).

### LoRaWAN Module

We use the Wio-E5 LoRa module, although any can be used with proper setup. Ensure that whichever transceiver being used is set to the proper data rate -- a specific RF band must be used depending on the region (For the US, 902MHz-928Mhz).

### LoRaWAN Gateway

The gateway you choose is dependent on a couple things, particularly connectivity options, OS, and region support. We use the Sentrius RG191, just ensure that the gateway supports your frequency spectrum.

### Network/Application Server

First off, I recommend reading about the difference between [network](https://www.thethingsindustries.com/docs/reference/components/network-server/) and [application](https://www.thethingsindustries.com/docs/reference/components/application-server/) servers. 

We use [ChirpStack](https://www.chirpstack.io/) to implement these.

In order to read data from the application server, you have to register your end device with that application. This can be done via either Over-the-Air (OTAA), or by personalization (ABP). We use ABP, in which you'll need the LoRaWAN MAC version, along with the device EUI. You'll be asked to enter the NwkSKey, AppSKey, and several other items. While it'll let you enter the factory default, it won't actually work. You have to set them yourself. See the  [commands](https://files.seeedstudio.com/products/317990687/res/LoRa-E5%20AT%20Command%20Specification_V1.0%20.pdf) to do so. To read more about either, check out [the TTN website](https://www.thethingsindustries.com/docs/devices/abp-vs-otaa/).

### System Assembly

For general gateway setup, refer to the [user manual](https://github.com/jlab-sensing/Rocketlogger-Firmware/blob/assets/pdf/RG1xx_User_Guide.pdf).

#### Having issues with initial gateway connection?
Something about eduroam makes connecting to the gateway troublesome so if you're stuck on it, just factory reset the gateway (instructions in the user manual). Then, if you can't connect to it via direct ethernet connection (PC<->Gateway), the Wi-Fi quick config method is reliable. You can then connect to whatever internet method you're using, and should be good to continue.

Once you've logged into the gateway, make sure you change the password under Settings->Password. If you plan on using the wifi configuration method, change the SoftAP password as well under Wi-Fi->Advanced.

To set the gateway up for LoRaWAN usage, go to LoRa->Forwarder and select "Semtech UDP Forwarder", then enter the server address (currently jlab.ucsc.edu). You can leave the ports alone. ![Forwarder Setup](https://github.com/jlab-sensing/Rocketlogger-Firmware/blob/assets/images/RG191_LORA_FORWARDER.png)

Then go to LoRa->Advanced, and check each of the "Forward CRC ..." options. ![Forward CRC ...](https://github.com/jlab-sensing/Rocketlogger-Firmware/blob/assets/images/RG191_LORA_ADVANCED.png)

For connecting to the internet via ethernet, you can just connect the ethernet cable and it should set itself up on its own. If you want to use Wi-Fi, you can connect to the intended network under Wi-Fi->Scan.

Check if it's working via the dashboard -- it'll show whether or not you're connected to LoRa/WiFi/Ethernet. Give the gateway at least 30 seconds to send a ping before the light turns green. ![Gateway Dashboard](https://github.com/jlab-sensing/Rocketlogger-Firmware/blob/assets/images/RG191_DASHBOARD.png)
