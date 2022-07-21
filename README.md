# BBLoraE5

The LoRa-E5 module that we're using communicates with the beaglebone over UART using AT commands. Because we're using the grove connector, we're
using UART2. If this were to be expanded and we were to use the standalone LoRa-E5 chip, any channel would work.

Notes: Right now, rc_uart_init doesn't work properly. All of the other functions seem to be working properly. I've been initializing the UART bus
using a python library just so I can get actual work done, rather than pigeonholing on the init function.

