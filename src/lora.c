#include "lora.h"
#include <string.h>

#define GENERIC_ERROR -1

#define TX 1
#define RX 2
#define VERIFY_RXTX(mode, res, len) {\
	if (res != len || res == -1){\
		return (mode == TX)?(TX_ERROR):(RX_ERROR); \
		}\
	}
	

int ATModule_TestConnection(){
	int i;
	int res = 0;
	char *outgoing = "AT\n";
	uint8_t incoming[MAX_PAYLOAD_LENGTH] = {0};
	
	//send message "AT" to test connection with e5 module
	int len = strlen(outgoing);
	res = rc_uart_write(UART2, (uint8_t*)outgoing, len);
	VERIFY_RXTX(TX, res, len);
	
	//wait 5ms to ensure module has time to respond. time is arbitrary
	rc_usleep(5000);
	
	//e-5 module should return "+AT: OK"
	char *resp = "+AT: OK"; //expected response
	len = strlen(resp);
	if (rc_uart_bytes_available > 0){
		res = rc_uart_read_line(UART2, incoming, len);
		VERIFY_RXTX(RX, res, len);
		
		//implement cleaner way to do this
		for (i=0;i<len;i++){
			if (incoming[i] != resp[i]){
				printf("\tError reading E5 module response: %s\n", (char*)&incoming);
				return RESPONSE_ERROR;
			}
		}
	// printf("\tCorrect module response: %s\n", (char*)&incoming);
	}
	return 0;
}


int ATModule_GetFWVer(){
	int res=0;
	char *outgoing = "AT+VER\n";
	uint8_t incoming[MAX_PAYLOAD_LENGTH] = {0};
	int len = strlen(outgoing);
	
	if (rc_uart_flush(UART2)){
		return GENERIC_ERROR;
	}
	
	//write "AT+VER" to E5 module
	res = rc_uart_write(UART2, (uint8_t*)outgoing, len);
	VERIFY_RXTX(TX, res , len);
	
	//wait 50ms, arbitrary
	rc_usleep(50000);
	
	if (rc_uart_bytes_available > 0){
		res = rc_uart_read_line(UART2, incoming, MAX_PAYLOAD_LENGTH);
		//unknown response length, so disregard length check in macro
		VERIFY_RXTX(RX, res, res); 
	}
	
	//instead of printing here, place it in an argument buffer
	printf("Firmware version: %s\n", (char*)&incoming);
	
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


int main(void){
	printf("\nBegin testing on %s at %s\n\n", __DATE__, __TIME__);
	

	if (rc_uart_init(2, 9600, 1, 0, 1, 0) == -1){
		printf("Error in UART2 initialization.\n");
	}
	
	if (ATModule_TestConnection()){
		printf("Beaglebone not connected to E5 module.\n");
	} else{ 
		printf("Beaglebone is connected.\n");
	}
	
	// if (ATModule_GetFWVer()){
	// 	printf("Error retrieving firmware version.\n");	
	// }
	
	char *outgoing = "AT+CMSG=\"balls\"\n";
	int len=strlen(outgoing);
	rc_uart_write(UART2, (uint8_t*)outgoing, len);
	
	return 0;
}
