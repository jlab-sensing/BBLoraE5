#include <stdio.h>
#include <stdlib.h>
#include <rc/uart.h>
#include <rc/time.h>

#define UART2 2 //uart2
#define BAUD96 9600
#define TIMEOUT 1 //1SEC
#define CAN_EN 0 //canonical mode enable
#define SB 1 //stop bits
#define PAR 0 //parity
#define MAX_PAYLOAD_LENGTH 528 //512 characters maximum


#define SUCCESS 0
#define ERROR -1

enum error_codes {
    TX_ERROR = 1,
    LENGTH_MISMATCH,
    RX_ERROR,
    RESPONSE_ERROR
};


// int Grove_Init(int bus, int baud, int timeout, int stop_bits, int parity);

int ATModule_Init();
int ATModule_TestConnection();
int ATModule_GetFWVer();