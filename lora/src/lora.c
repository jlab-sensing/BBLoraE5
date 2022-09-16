#include "lora.h"

#include <string.h>
#include <stdint.h>
#include <unistd.h>

#define AT_TEST_HARNESS
	
//Generic function to send arbitrary payload
int AT_SerialTransmit(int bus, char *data){
	if ((bus > 5) || (bus < 0)) return BAD_BUS;

	int res = 0;
	int len = strlen(data);
	
	//empty bus of any remaining data
	if (rc_uart_flush(bus)){
		return ERROR;
	}
	//write data to uart bus
	res = rc_uart_write(bus, data, len);
	if (res != len) return TX_ERROR;
	
	return SUCCESS;
}

//Generic function to retrieve payload
int AT_SerialReceive(int bus, char * buf){
	if ((bus > 5) || (bus < 0)) return BAD_BUS;
	int res = 0;
	
	res = rc_uart_read_line(bus, buf, MAX_PAYLOAD_LENGTH);
	//unknown response length, so disregard length check in macro
	if (res < 0) return RX_ERROR;
	
	return SUCCESS;
}

//Sends a dummy message to LoRa module to verify connectivity
int AT_TestConnection(int bus){
	if ((bus > 5) || (bus < 0)) return BAD_BUS;
	int i;
	char incoming[MAX_PAYLOAD_LENGTH] = {0};
	
	//send message "AT" to test connection with e5 module
	if (AT_SerialTransmit(bus, "AT\n")){return TX_ERROR;}
	
	//e-5 module should return "+AT: OK"
	char resp[] = "+AT: OK"; //expected response
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

//Retrieves current LoRaWAN version
int AT_CheckVersion(int bus){
	if ((bus > 5) || (bus < 0)) return BAD_BUS;
	char incoming[MAX_PAYLOAD_LENGTH] = {0};

	//Retrieve module firmware version
	if (AT_SerialTransmit(bus, "AT+VER\n")){
		return TX_ERROR;
	}
	
	if (AT_SerialReceive(bus, incoming)) {
		return RX_ERROR;
	}
	
	//tbd: instead of printing here, maybe place it in an argument buffer?
	//printf("Firmware version: %s\n", incoming);
	
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

//Retrieves current module ID information: DevAddr, DevEui, AppEui
int AT_CheckID(int bus){
	if ((bus > 5) || (bus < 0)) return BAD_BUS;
	int i;
	char incoming[MAX_PAYLOAD_LENGTH];
	
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

//Prints the module's current data rate settings
int AT_CheckDataRate(int bus){
	if ((bus > 5) || (bus < 0)) return BAD_BUS;
	char buf[MAX_PAYLOAD_LENGTH] = {0};
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

//Set module network session key -- required for application registration
int AT_SetNwkSKey(int bus, long long key){
	if ((bus > 5) || (bus < 0)) return BAD_BUS;
	//need to ensure that network session key is proper length (16bytes)
	
	char data[SKEY_MSG_LEN];
	
	//place string into buffer
	snprintf(data, SKEY_MSG_LEN, "AT+KEY=NWKSKEY, \"%lli\"\n", key);
	if (AT_SerialTransmit(bus, data)){return TX_ERROR;}
	
	//currently no check to see if correct response
	//tbd: maybe make the return value the response key from the e5 module

	return SUCCESS;
}

//Set application session key -- required for application registration
int AT_SetAppSKey(int bus, long long key){
	if ((bus > 5) || (bus < 0)) return BAD_BUS;
	//need to ensure that application session key is proper length (16bytes)
	
	char data[SKEY_MSG_LEN];
	
	//place string into buffer
	snprintf(data, SKEY_MSG_LEN, "AT+KEY=APPSKEY, \"%lli\"\n", key);
	if (AT_SerialTransmit(bus, data)){return TX_ERROR;}
	
	//currently no check to see if correct response
	//tbd: maybe make the return value the response key from the e5 module

	return SUCCESS;
}

//Set module data rate
int AT_SetDataRate(int bus, int rate){
	if ((bus > 5) || (bus < 0)) return BAD_BUS;
	if ((rate<0) || (rate > 15)){return ERROR;}
	
	char data[MAX_PAYLOAD_LENGTH];
	char buf[MAX_PAYLOAD_LENGTH] = {0};
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

//Put LoRaWAN module into low power mode
int AT_LowPower(int bus, int timeout){
	if ((bus > 5) || (bus < 0)) return BAD_BUS;
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

//Send a generic string over LoRa module
int AT_SendString(int bus, char *str){
	if ((bus > 5) || (bus < 0)) return BAD_BUS;
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

//Initialize LoRaWAN module
int AT_Init(int bus, int baud, int timeout){
	char init_msg[MAX_PAYLOAD_LENGTH] = {0};
	sprintf(init_msg, "stty -F /dev/ttyO%i 9600 cs8 -cstopb -parenb", bus);
	system(init_msg);
	
	if (rc_uart_init(bus, baud, timeout, CAN_EN, SB, PAR) == -1){
		printf("Error in UART2 initialization.\n");
		return ERROR;
	}
	//if you have a serial terminal open (say you want to directly
	//read the module's responses, you won't be able to read them from
	//this program. meaning you might enter this if statement, but it
	//actually is connected.)
	if (AT_TestConnection(bus)){
		printf("Beaglebone not connected to E5 module.\n");
	}
	//ADR will automatically set data rates, but we want to stick with DR2
	//Not necessary for initialization, up to user
	if (AT_SerialTransmit(bus, "AT+ADR=OFF\n") == -1){
		printf("Error setting ADR function.\n");
		return ERROR;
	}
	//I don't like having the sleeps here, but if you send two commands
	//too quickly, they'll essentially merge and both will fail.
	sleep(1);
	if (AT_SerialTransmit(bus, "AT+CH=NUM,8-15,64\n") == -1){
		printf("Error selecting channels.\n");
		return ERROR;
	}
	sleep(1);
	if (AT_SetDataRate(bus, 2) == -1){
		printf("Error setting datarate.\n");
		return ERROR;
	}
	
	return SUCCESS;
}
