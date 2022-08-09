#include "lora.h"
#include <string.h>

#define AT_TEST_HARNESS


#define TX 1
#define RX 2
#define VERIFY_RXTX(mode, res, len) {\
	if (res != len || res == -1){\
		return (mode == TX)?(TX_ERROR):(RX_ERROR); \
		}\
	}
	
#define VERIFY_BUS(bus) if ((bus > 5) || (bus < 0)){return BAD_BUS;}


//Ensure that string isn't too long
int AT_SerialTransmit(int bus, char *data){
	VERIFY_BUS(bus);
	int res = 0;
	int len = strlen(data);
	
	//empty bus of any remaining data
	if (rc_uart_flush(bus)){
		return ERROR;
	}
	//write data to uart bus
	res = rc_uart_write(bus, (uint8_t*)data, len);
	VERIFY_RXTX(TX, res , len);
	
	return SUCCESS;
}

int AT_SerialReceive(int bus, uint8_t buf[]){
	VERIFY_BUS(bus);
	int res = 0;
	
	if (rc_uart_bytes_available > 0){
		res = rc_uart_read_line(bus, buf, MAX_PAYLOAD_LENGTH);
		//unknown response length, so disregard length check in macro
		VERIFY_RXTX(RX, res, res); 
	} else{
		return ERROR;
	}
	
	return SUCCESS;
}

int AT_TestConnection(int bus){
	VERIFY_BUS(bus);
	int i;
	uint8_t incoming[MAX_PAYLOAD_LENGTH] = {0};
	
	//send message "AT" to test connection with e5 module
	if (AT_SerialTransmit(bus, "AT\n")){return TX_ERROR;}
	
	//e-5 module should return "+AT: OK"
	char *resp = "+AT: OK"; //expected response
	int len = strlen(resp);
	if (AT_SerialReceive(bus, incoming)){return RX_ERROR;}
		
	//implement cleaner way to do this
	for (i=0;i<len;i++){
		if (incoming[i] != resp[i]){
			printf("\tError reading E5 module response: %s\n", (char*)&incoming);
			return ERROR;
		}
	}
	
	return SUCCESS;
}

int AT_CheckVersion(int bus){
	VERIFY_BUS(bus);
	uint8_t incoming[MAX_PAYLOAD_LENGTH] = {0};

	//Retrieve module firmware version
	if (AT_SerialTransmit(bus, "AT+VER\n")){
		return TX_ERROR;
	}
	
	if (AT_SerialReceive(bus, incoming)) {
		return RX_ERROR;
	}
	
	//tbd: instead of printing here, maybe place it in an argument buffer?
	printf("Firmware version: %s\n", (char*)&incoming);
	
	//Retrieve LoRaWAN version
	if (AT_SerialTransmit(bus, "AT+LW=VER\n")) {
		return TX_ERROR;
	}
	
	if (AT_SerialReceive(bus, incoming)) {
		return RX_ERROR;
	}
	
	printf("LoRaWAN version: %s\n", (char*)&incoming);
	return SUCCESS;
}

int AT_CheckID(int bus){
	VERIFY_BUS(bus);
	int i;
	uint8_t incoming[MAX_PAYLOAD_LENGTH];
	
	if (AT_SerialTransmit(bus, "AT+ID\n")) {
		return TX_ERROR;
	}
	
	//Print DevAddr, DevEui, AppEui
	//would like to use while loop to retrive data until empty but it loops
	//and i don't feel like dealing with it right now
	//H A R D   C O D E D
	for (i=0;i<3;i++){
		if (AT_SerialReceive(bus, incoming)){return RX_ERROR;}
		printf("Device info: %s\n", incoming);
	}
	
	return SUCCESS;
}

int AT_CheckDataRate(int bus){
	VERIFY_BUS(bus);
	uint8_t buf[MAX_PAYLOAD_LENGTH] = {0};
	//request device to send data rate
	if (AT_SerialTransmit(bus, "AT+DR\n")){return TX_ERROR;}

	//collect response from AT module
	/*I read from the UART bus twice because the response consists of two parts:
	*	>DR0
	*	>+DR: US915 DR0  SF10 BW125K    (numbers may be different)
	* We get the info from the first in the second response, so I just throw
	* away the first. It's a lazy method, but it works*/
	if(AT_SerialReceive(bus, buf)){return RX_ERROR;}
	if(AT_SerialReceive(bus, buf)){return RX_ERROR;}
	printf("Current datarate: %s\n", buf);
	return SUCCESS;
}

int AT_SetNwkSKey(int bus, uint8_t *key){
	VERIFY_BUS(bus);
	//need to ensure that network session key is proper length (16bytes)
	
	char data[SKEY_MSG_LEN];
	
	//place string into buffer
	snprintf(data, SKEY_MSG_LEN, "AT+KEY=NWKSKEY, \"%s\"\n", key);
	if (AT_SerialTransmit(bus, data)){return TX_ERROR;}
	
	//currently no check to see if correct response
	//tbd: maybe make the return value the response key from the e5 module

	return SUCCESS;
}

int AT_SetAppSKey(int bus, uint8_t *key){
	VERIFY_BUS(bus);
	//need to ensure that application session key is proper length (16bytes)
	
	char data[SKEY_MSG_LEN];
	
	//place string into buffer
	snprintf(data, SKEY_MSG_LEN, "AT+KEY=APPSKEY, \"%s\"\n", key);
	if (AT_SerialTransmit(bus, data)){return TX_ERROR;}
	
	//currently no check to see if correct response
	//tbd: maybe make the return value the response key from the e5 module

	return SUCCESS;
}

int AT_SetDataRate(int bus, int rate){
	VERIFY_BUS(bus);
	if ((rate<0) || (rate > 15)){return ERROR;}
	
	char data[MAX_PAYLOAD_LENGTH];
	uint8_t buf[MAX_PAYLOAD_LENGTH] = {0};
	snprintf(data, 11, "AT+DR=dr%i\n", rate);
	
	if (AT_SerialTransmit(bus, data)){
		return TX_ERROR;
	}
	
	if (AT_SerialReceive(bus, buf)) {
		return RX_ERROR;
	}
	
	if (AT_SerialReceive(bus, buf)) {
		return RX_ERROR;
	}
	
	printf("New data rate: %s\n", buf);
	return SUCCESS;
}

int AT_LowPower(int bus, int timeout){
	VERIFY_BUS(bus);
	if (timeout<0)return ERROR;
	
	if (timeout){
		char data[MAX_PAYLOAD_LENGTH] = {0};
		snprintf(data, MAX_PAYLOAD_LENGTH, "AT+LOWPOWER=%i\n", timeout);
		AT_SerialTransmit(bus, data);
	} else{
		AT_SerialTransmit(bus, "AT+LOWPOWER\n");
	}
	return SUCCESS;
}

int AT_SendString(int bus, char *str){
	VERIFY_BUS(bus);
	char data[MAX_PAYLOAD_LENGTH] = {0};

	if (snprintf(data, MAX_PAYLOAD_LENGTH, "AT+CMSG=\"%s\"\n", str) < 0){
		printf("Error inserting message string.\n");
		return ERROR;
	}
	if (AT_SerialTransmit(bus, data)){
		printf("Error transmitting message data.\n");
		return TX_ERROR;
	}
	// printf("Sent string: %s\n", data);
	return SUCCESS;
	
}

int AT_Init(void){
	
	system("config-pin P9.21 uart\n");
	system("config-pin P9.22 uart\n");
	system("stty -F /dev/ttyS2 9600 cs8 -cstopb -parenb\n");
	
	if (rc_uart_init(UART2, 9600, 1, CAN_EN, SB, PAR) == -1){
		printf("Error in UART2 initialization.\n");
		// return ERROR;
	}
	
	if (AT_TestConnection(UART2)){
		printf("Beaglebone not connected to E5 module.\n");
		// return ERROR;
	}
	//ADR will automatically set data rates, but we want to stick with DR1
	//Not necessary for initialization, up to user
	if (AT_SerialTransmit(UART2, "AT+ADR=OFF\n") == -1){
		printf("Error setting ADR function.\n");
	}
	
    // rc_uart_write(UART2, (uint8_t*)"AT+DR=dr2\n", strlen("AT+DR=2\n"));

	return SUCCESS;
}