# BBLoraE5

Notes: The LoRa-E5 module that we're using communicates with the beaglebone over UART using AT commands. Because we're using the grove connector, we're
using UART2. If this were to be expanded and we were to use the standalone LoRa-E5 chip, any channel would work.

## Connection/Connection Issues

Make sure that the module is connected properly with the TestConnection function. Keep in mind that if you have a serial terminal open, function
calls may return false despite actually having worked. Most relevant info is printed to the CLI, but if you want to see all of the module's
responses, open a serial terminal with
```minicom -b 9600 -D /dev/ttyO*```
LoRa-E5 modules default to a baudrate of 9600 (although if stuck in the bootloader, they'll send repeated "c's" at a speed of 115200). The grove
module that we're currently using is connected to UART2 (thus ttyO2).

Right now, rc_uart_init doesn't work properly. All of the other functions seem to be working properly. I've been initializing the UART bus
using a python library just so I can get actual work done, rather than pigeonholing on the init function. Until I can get it to work, the
easiest option is to just do 
```
~$ python
    >>> import Adafruit_BBIO.UART as UART
    >>> UART.setup("UART2")
    >>> quit()
```

## Keys/Activation

Joining an application server OTAA is recommended, but hasn't worked thus far. Until I can figure out why OTAA activation won't work, we're stuck
using ABP.

To activate LoRaWAN devices using ABP, you'll need 
-LoRaWAN MAC version
-Device EUI
-Network Session Key
-Application Session Key


Each LoRa-E5 module provides deafult values for the AppKey, AppSKey, NwkSKey, and AppEui. I recommend generating new (random) values for each
of these and setting them as soon as possible. These defaults are the same for **each module**, so it's really important to set new ones.

You can get generic device ID through a call to CheckID.

The default LoRaWAN MAC version is V1.0.2B, but some modules make it through with different versions. I came across one running V1.0.3,
and it caused some activation issues. Make sure that you check yourself, rather than blindly trusting the default settings.


## Data Rate Options

LoRaWAN is based on the Chirp Spread Spectrum technique, and relies on something called the spreading factor. The spreading factor affects
several characteristics such as data rate, distance, airtime, sensitivity, battery life, and the maximum message length.

Increasing spreading factor-->higher bit rate, further distance, more airtime, higher receiver sensitivity, shorter battery life.

End devices which are close to gateways should use a lower spreading factor and higher data rate, while devices further away should use a 
high spreading factor because they need a higher link budget.

