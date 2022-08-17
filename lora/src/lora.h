#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "uart.h"

#define UART2 2 //uart2
#define BAUD96 9600
#define TIMEOUT 1 //1SEC
#define CAN_EN 0 //canonical mode enable
#define SB 1 //stop bits
#define PAR 0 //parity
#define DATA_RATE 1

#define MAX_PAYLOAD_LENGTH 126 //53 characters maximum
#define SKEY_MSG_LEN 52

#define SUCCESS 0
#define ERROR -1

enum error_codes {
    TX_ERROR = 1,
    RX_ERROR,
    BAD_BUS,
    BAD_BAUD,
    BAD_TIMEOUT
};


// int Grove_Init(int bus, int baud, int timeout, int stop_bits, int parity);

int AT_Init(void);
int AT_SerialTransmit(int bus, char *data);
int AT_SerialReceive(int bus, char * buf);
int AT_TestConnection(int bus);
int AT_CheckVersion(int bus);
int AT_CheckID(int bus);
int AT_CheckDataRate(int bus);
int AT_SetNwkSKey(int bus, uint8_t *key);
int AT_GetNwkSKey(int bus);
int AT_SetAppSKey(int bus, uint8_t *key);
int AT_GetAppSKey(int bus);
int AT_SetDataRate(int bus, int rate);
int AT_LowPower(int bus, int timeout);
int AT_SendString(int bus, char *str);

