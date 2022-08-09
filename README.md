Overview
===================================================================================================

DirtViz is a project intended to facilitate the evaluation of microbial fuel cells. Because they're typically placed outside of lab settings,
it's difficult to collect data from cells without going to their physical location and taking data from an SD card. As an alternative to this, we've 
implemented LoRaWAN to broadcast data collected by evaluation devices in the field. The data is received by a LoRaWAN gateway, which can be bought[link]
or built[link] depending on the situation. It's then forwarded to our network/application server (we use chirpstack[link], but there are several
alternatives such as TTN[link]), where the payload is processed and then uploaded to a database. We then plot this data to provide an easily understood,
graphical representation of the MFC's performance.

LoRaWAN
----------------------------------------------------------------------------------------------------

Specifics on LoRaWAN and how it works can be found here[link].

###LoRa Transceiver

We use the Grove Wio-E5 LoRa[link] module, although any can be used with proper setup. Ensure that whichever transceiver being used is set to the 
proper data rate -- a specific RF band must be used depending on the region.

###Network/Application Server

How you decide to implement your network/application server may depend on 
