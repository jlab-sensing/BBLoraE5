#include "lora.h"
#include <string.h>

#define AT_TEST_HARNESS

#define GENERIC_ERROR -1

#define TX 1
#define RX 2
#define VERIFY_RXTX(mode, res, len) {\
	if (res != len || res == -1){\
		return (mode == TX)?(TX_ERROR):(RX_ERROR); \
		}\
	}
	
#define VERIFY_BUS(bus) if ((bus > 5) || (bus < 0)){return IMPROPER_BUS_VALUE;}

	
int ATModule_SerialTransmit(int bus, const char *data){
	VERIFY_BUS(bus);
	int res = 0;
	int len = strlen(data);
	
	//empty bus of any remaining data
	if (rc_uart_flush(bus)){
		return GENERIC_ERROR;
	}
	//write data to uart bus
	res = rc_uart_write(bus, (uint8_t*)data, len);
	VERIFY_RXTX(TX, res , len);
	
	return 0;
}

int ATModule_SerialReceive(int bus, uint8_t buf[]){
	VERIFY_BUS(bus);
	int res = 0;
	
	if (rc_uart_bytes_available > 0){
		res = rc_uart_read_line(bus, buf, MAX_PAYLOAD_LENGTH);
		//unknown response length, so disregard length check in macro
		VERIFY_RXTX(RX, res, res); 
	} else{
		return EMPTY_BUS;
	}
	
	return 0;
}

int ATModule_TestConnection(int bus){
	VERIFY_BUS(bus);
	int i;
	uint8_t incoming[MAX_PAYLOAD_LENGTH] = {0};
	
	//send message "AT" to test connection with e5 module
	if (ATModule_SerialTransmit(bus, "AT\n")){return TX_ERROR;}
	
	//wait 50ms, arbitrary, maybe not even needed
	rc_usleep(5000);
	
	//e-5 module should return "+AT: OK"
	char *resp = "+AT: OK"; //expected response
	int len = strlen(resp);
	if (ATModule_SerialReceive(bus, incoming)){return RX_ERROR;}
		
	//implement cleaner way to do this
	for (i=0;i<len;i++){
		if (incoming[i] != resp[i]){
			printf("\tError reading E5 module response: %s\n", (char*)&incoming);
			return RESPONSE_ERROR;
		}
	}
	return 0;
}

int ATModule_CheckVersion(int bus){
	VERIFY_BUS(bus);
	uint8_t incoming[MAX_PAYLOAD_LENGTH] = {0};

	//Retrieve module firmware version
	if (ATModule_SerialTransmit(bus, "AT+VER\n")){return TX_ERROR;}
	rc_usleep(5000);	//wait 50ms, arbitrary, maybe not even needed
	if (ATModule_SerialReceive(bus, incoming)){return RX_ERROR;}
	
	//tbd: instead of printing here, maybe place it in an argument buffer?
	printf("Firmware version: %s\n", (char*)&incoming);
	
	//Retrieve LoRaWAN version
	if (ATModule_SerialTransmit(bus, "AT+LW=VER\n")){return TX_ERROR;}
	rc_usleep(5000);
	if (ATModule_SerialReceive(bus, incoming)){return RX_ERROR;}
	
	printf("LoRaWAN version: %s\n", (char*)&incoming);
	return 0;
}

int ATModule_CheckID(int bus){
	VERIFY_BUS(bus);
	int i;
	uint8_t incoming[MAX_PAYLOAD_LENGTH];
	
	if (ATModule_SerialTransmit(bus, "AT+ID\n")){return TX_ERROR;}
	
	rc_usleep(5000);//again, maybe not necessary but for peace of mind
	
	//Print DevAddr, DevEui, AppEui
	//would like to use while loop to retrive data until empty but it loops
	//and i don't feel like dealing with it right now
	//H A R D   C O D E D
	for (i=0;i<3;i++){
		if (ATModule_SerialReceive(bus, incoming)){return RX_ERROR;}
		printf("Device info: %s\n", incoming);
	}
	
	return 0;
}

int ATModule_SetNwkSKey(int bus, uint8_t *key){
	VERIFY_BUS(bus);
	//need to ensure that network session key is proper length (16bytes)
	
	char data[SKEY_MSG_LEN];
	//place string into buffer
	snprintf(data, SKEY_MSG_LEN, "AT+KEY=NWKSKEY, \"%s\"\n", key);
	if (ATModule_SerialTransmit(bus, data)){return TX_ERROR;}
	
	//currently no check to see if correct response
	//tbd: maybe make the return value the response key from the e5 module

	return 0;
}

int ATModule_SetAppSKey(int bus, uint8_t *key){
	VERIFY_BUS(bus);
	//need to ensure that application session key is proper length (16bytes)
	
	char data[SKEY_MSG_LEN];
	//place string into buffer
	snprintf(data, SKEY_MSG_LEN, "AT+KEY=APPSKEY, \"%s\"\n", key);
	if (ATModule_SerialTransmit(bus, data)){return TX_ERROR;}
	
	//currently no check to see if correct response
	//tbd: maybe make the return value the response key from the e5 module

	return 0;
}




// int GroveModule_Init(){
// 	int res = 0;
// 	unsigned int i;
// 	uint8_t flag=0;
	
// 	char data_out[MAX_PAYLOAD_LENGTH] = "AT\n";
// 	uint8_t data_in[MAX_PAYLOAD_LENGTH];
// 	int len = strlen(data_out);
	
// 	//check data rate scheme
// 	// data_out = "AT+DR=SCHEME";
// 	strcpy(data_out, "AT+DR=SCHEME");
// 	len = strlen(data_out);
// 	if (rc_uart_write(UART2, (uint8_t*)data_out, len) == ERROR){
// 		printf("Error sending message to check data rate scheme.\n");
// 		return ERROR;
// 	}
// 	rc_usleep(5000);
// 	strcpy(resp, "AT+DR: US915");
// 	len = strlen(resp);
// 	if (rc_uart_read_line(UART2, data_in, len) == ERROR){
// 		printf("Error reading data rate response.\n");
// 	}
// 	for (i=0;i<len;i++){
// 		if (data_in[i] != resp[i]){
// 			flag = 1;
// 		}
// 	}
// 	printf("Current data rate: %s\n", &data_in);

	
// 	// if (flag){
// 	// 	//set data rate scheme: US915
// 	// 	*data_out = "AT+DR=US915";
// 	// 	len=strlen(data_out);
// 	// 	if (rc_uart_write(UART2, data_out, len) == ERROR){
// 	// 		printf("Error sending \"Set data scheme\" message.\n);
// 	// 	}
// 	// }
// 	return SUCCESS;
// }

#ifdef AT_TEST_HARNESS

int main(void){
	printf("\nBegin testing on %s at %s\n\n", __DATE__, __TIME__);
	

	if (rc_uart_init(2, 9600, 1, 0, 1, 0) == -1){
		printf("Error in UART2 initialization.\n");
	}
	
	if (ATModule_TestConnection(UART2)){
		printf("Beaglebone not connected to E5 module.\n");
	} else{ 
		printf("Beaglebone is connected.\n");
	}
	
	if (ATModule_CheckVersion(UART2)){
		printf("Error retrieving version.\n");	
	}
	
	/* Commented out because I don't want to set the network session key each
	/  time I test functions. I verified that it works with our current key.
	/  Because SetNwkSKey works, SetAppSKey should too since they're essentially
	/  identical.
	*/
	// char *nwkskey = "bf6eaef13678c9d708b1f8fd9db1b710";
	// if (ATModule_SetNwkSKey(UART2, (uint8_t*)nwkskey)){
	// 	printf("Error setting NwkSkey.\n");
	// }
	
	if (ATModule_CheckID(UART2)){
		printf("Error retrieving device ID info.\n");
	}
	
	return 0;
}

#endif
