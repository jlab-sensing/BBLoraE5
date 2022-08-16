Overview
===================================================================================================

DirtViz is a project intended to facilitate the evaluation of microbial fuel cells. Because they're typically placed outside of lab settings,
it's difficult to collect data from cells without going to their physical location and taking data from an SD card. As an alternative to this, we've 
implemented LoRaWAN to broadcast data collected by evaluation devices in the field. The data is received by a LoRaWAN gateway, which can be bought[link]
or built[link] depending on the situation. It's then forwarded to our network/application server (we use chirpstack[link], but there are several
alternatives such as The Things Network (TTN)[link]), where the payload is processed and then uploaded to a database. We then plot this data to provide an easily understood,
graphical representation of the MFC's performance.

LoRaWAN
----------------------------------------------------------------------------------------------------

Specifics on LoRaWAN and how it works can be found here[link].

### LoRaWAN Module

We use the Grove Wio-E5 LoRa[link] module, although any can be used with proper setup. Ensure that whichever transceiver being used is set to the proper data rate -- a specific RF band must be used depending on the region (For the US, 902MHz-928Mhz).

### LoRaWAN Gateway

The gateway you choose is dependent on a couple things, particularly connectivity options, OS, and region support. We use the Sentrius RG191, just ensure that whichever gateway you use supports your frequency spectrum.

### Network/Application Server

First off, I recommend reading about the difference between [network](https://www.thethingsindustries.com/docs/reference/components/network-server/) and [application](https://www.thethingsindustries.com/docs/reference/components/application-server/) servers.

How you decide to implement your network/application server may depend on which gateway you implement (some gateways have native support for services such as TTN). I recommend either ChirpStack or TTN. TTN is better for initial proof of concept, but ChirpStack offers more customization/integrations, and is better for actual deployment.

### System Assembly

First, configure the gateway. You'll have to either connect the gateway directly to ethernet or to your computer to begin. For the RG191, you can access the gateway through https://rg1xx2**9378B**.local, where the text in bold is the last six digits of the ethernet MAC ID. The gateway should come with a user manual with further details regarding setup.

With the gateway connected to the internet, we can now set up LoRaWAN capabilities. On the gateway interface, there's a LoRa page where you can configure relevant settings. Specifics will depend on how you decide to implement the network and application servers. Some gateways provide presets which make configuration easy (the RG191, for example, offers a TTN preset).

Using the TTN preset makes setup quick and easy, but less flexible. I would recommend setting it up using either the [Semtech UDP forwarder](https://www.thethingsindustries.com/docs/gateways/udp/) or [LoRa Basics Station](https://www.thethingsindustries.com/docs/gateways/lora-basics-station/). You'll find these options under the "Forwarder" tab. There's an option to set your network server address. If you use TTN, they have dedicated servers for each region which can be found in their documentation (it should be set by default if you use their preset). For most other options though, you'll have to set up your own server to use.

You should't have to change much beyond what's described in the provided links, although I found that the Semtech UDP forwarder might not work if you
don't ensure that each of the "Forward CRC ******" options listed under aren't checked "Advanced" aren't checked (this is specifically for the RG191, I
can't speak for other gateways).

If you're using TTN, setting up your application server is fairly straightforward, as they provide most of the info. ChirpStack isn't particularly
complicated either, but is slightly more confusing. Assuming you have your network server up and running, you can connect it to ChirpStack under the
Network-Servers tab by providing a name and the server.

In order to read data from the application server, you have to register your end device with that application. This can be done either Over-the-Air (OTAA), or by personalization (ABP). We use ABP, in which you'll need the LoRaWAN MAC version, along with the device EUI. To read more about either, check out [the TTN website](https://www.thethingsindustries.com/docs/devices/abp-vs-otaa/).
