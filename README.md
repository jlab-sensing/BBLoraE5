Overview
===================================================================================================

This repo contains the firmware used to measure and upload MFC data to our visualization portal, [DirtViz](https://dirtviz.jlab.ucsc.edu). The measurement itself is done with a [RocketLogger](https://www.rocketlogger.ethz.ch/), and either sent to our database via ethernet or [LoRaWAN](https://lora-alliance.org/about-lorawan/). 

[Link to the Dirtviz Repo](https://github.com/jlab-sensing/DirtViz)

Note: This project is based on v1 of the RocketLogger firmware -- our code has not yet been tested on v2.

Configuring the Rocketloggers
---------------------------------------------------------------------------------------------------

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

Running the logging program
----------------------------------------------------------------------------------------------------

Before you collect any data, you must set some configurations. First, set the number of samples to take inside of the "logger" script (located in the scripts folder). The Rocketlogger collects 1 sample per second, and the Teros sensor 1 sample per 10 seconds. When you set the variable `NUM_SAMPLES`, it refers to the number of Rocketlogger samples. The default is 30 (I don't recommend going lower than this), though I recommend setting it to several minutes.

Enter `rl.conf` and set the data transmission method (simply "ethernet" or "lora") and cell names. Default is ethernet. It sends via POST request, if you need to change the address you can do so in `main.c`.

With the configurations set, build the project with `sudo ./install.sh`. Use a screen session so that you can detach from the program with `screen -S session-name`. Run the program with the "logger" script, then when you're ready to leave, hit `ctrl-a` then `ctrl-d`, and you can log out with the program still running.

LoRaWAN
----------------------------------------------------------------------------------------------------

Specifics on LoRaWAN and how it works can be found [here](https://lora-alliance.org/about-lorawan/).

### LoRaWAN Module

We use the Wio-E5 LoRa module, although any can be used with proper setup. Ensure that whichever transceiver being used is set to the proper data rate -- a specific RF band must be used depending on the region (For the US, 902MHz-928Mhz).

### LoRaWAN Gateway

The gateway you choose is dependent on a couple things, particularly connectivity options, OS, and region support. We use the Sentrius RG191, just ensure that the gateway supports your frequency spectrum.

### Network/Application Server

First off, I recommend reading about the difference between [network](https://www.thethingsindustries.com/docs/reference/components/network-server/) and [application](https://www.thethingsindustries.com/docs/reference/components/application-server/) servers. 

We use [ChirpStack](https://www.chirpstack.io/) to implement these.

### System Assembly

To configure the gateway, you'll have to either connect the gateway directly to your computer. For the RG191, you can access the gateway through https://rg1xx2**9378B**.local, where the text in bold is the last six digits of the ethernet MAC ID. See the [user guide](https://www.lairdconnect.com/documentation/user-guidedatasheet-rg1xx-rg191lte) for more specifics on general configuration.

With the gateway connected to the internet, we can now set up LoRaWAN capabilities. On the gateway interface, there's a LoRa page where you can configure relevant settings. Specifics will depend on how you decide to implement the network and application servers. Some gateways provide presets which make configuration easy (the RG191, for example, offers a TTN preset). I would recommend setting it up using either the [Semtech UDP forwarder](https://www.thethingsindustries.com/docs/gateways/udp/) or [LoRa Basics Station](https://www.thethingsindustries.com/docs/gateways/lora-basics-station/). You'll find these options under the "Forwarder" tab.

You should't have to change much beyond what's described in the provided links, although I found that the Semtech UDP forwarder might not work if you
don't ensure that each of the "Forward CRC ******" options listed under aren't checked "Advanced" aren't checked (this is specifically for the RG191, I
can't speak for other gateways).

If you're using TTN, setting up your application server is fairly straightforward, as they provide most of the info. ChirpStack isn't particularly
complicated either, but is slightly more confusing. Assuming you have your network server up and running, you can connect it to ChirpStack under the
Network-Servers tab by providing a name and the server.

In order to read data from the application server, you have to register your end device with that application. This can be done either Over-the-Air (OTAA), or by personalization (ABP). We use ABP, in which you'll need the LoRaWAN MAC version, along with the device EUI. You'll be asked to enter the NwkSKey, AppSKey, and several other items. While it'll let you enter the factory default, it won't actually work. You have to set them yourself. See the  [commands](https://files.seeedstudio.com/products/317990687/res/LoRa-E5%20AT%20Command%20Specification_V1.0%20.pdf) to do so. To read more about either, check out [the TTN website](https://www.thethingsindustries.com/docs/devices/abp-vs-otaa/).
