/*******************************************************************************
 * Created by: Brian Govers
 * Name: main.c
 * Date of Creation: 27 July 2022
 ******************************************************************************/
 
 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <string.h>
 #include "lora.h"
 #include "csv.h"
 
 //#define TESTING
 #define T_LORA
 //#define T_CSV
 
 #define BUFFER_SIZE 1024
 #define NUM_RL_FIELDS 6
 #define NUM_SAMPLES 500
 #

enum data_fields{TIMESTAMP = 0, I1LV, I2LV, I1H, I1L, V1, V2, I2H, I2L};

struct rl_samples{
 	uint8_t I1L_Valid, I2L_Valid;
	int rl_data[NUM_RL_FIELDS];
};

static uint8_t col = 0;
 
void cb1 (void *s, size_t len, void *data){
	int chr = 0;
	chr = strtol((char*)s, NULL, 10);
	//quick n easy, change later
	if (col == I1LV) ((struct rl_samples*)data)->I1L_Valid = chr;
	else if (col == I2LV) ((struct rl_samples*)data)->I2L_Valid = chr;
	else if (col == I1H) ((struct rl_samples*)data)->rl_data[0] = chr;//+= chr/NUM_SAMPLES;
	else if (col == I1L) ((struct rl_samples*)data)->rl_data[1] = chr;
	else if (col == V1) ((struct rl_samples*)data)->rl_data[2] = chr;
	else if (col == V2) ((struct rl_samples*)data)->rl_data[3] = chr;
	else if (col == I2H) ((struct rl_samples*)data)->rl_data[4] = chr;
	else if (col == I2L) ((struct rl_samples*)data)->rl_data[5] = chr;
	
	col++;
}
void cb2 (int c, void *data){
	col = 0;
}
 
 
 
int main(void){
	printf("\nProgram compiled on %s at %s\n\n", __DATE__, __TIME__);
	
	if (rc_uart_init(2, 9600, 1, 0, 1, 0) == -1){
		printf("Error in UART2 initialization.\n");
		// exit(EXIT_FAILURE);
	}
	if (AT_TestConnection(UART2)){
		printf("Beaglebone not connected to E5 module.\n");
		// exit(EXIT_FAILURE);
	}
	
	if (AT_SetDataRate(UART2, 1)){
		printf("Error setting device data rate.\n");
	}

	sleep(1);
	char *tx2 = "AT+ADR=OFF\n";
	AT_SerialTransmit(UART2, tx2);
	
	FILE *fp;
    struct csv_parser p;
    char buf[BUFFER_SIZE];
    size_t bytes_read;
	struct rl_samples rl = {0};
	char trx[MAX_PAYLOAD_LENGTH] = {0};

    if (csv_init(&p, 0) != 0){
        printf("init fail\n");
        exit(EXIT_FAILURE);
    } 
    fp = fopen("samples/rocketlogger.csv", "r");
    if (!fp){
        printf("fopen fail\n");
        exit (EXIT_FAILURE);
    } 
    csv_set_opts(&p, CSV_APPEND_NULL);
    while ((bytes_read=fread(buf, 1, 1024, fp)) > 0){
        if (csv_parse(&p, buf, bytes_read, cb1, cb2, &rl) != bytes_read) {
            fprintf(stderr, "Error while parsing file: %s\n",
            csv_strerror(csv_error(&p)) );
            exit(EXIT_FAILURE);
        }
    	sprintf(trx, "%i,%i,%i,%i,%i,%i", rl.rl_data[0], rl.rl_data[1], \
    		rl.rl_data[2], rl.rl_data[3], rl.rl_data[4], rl.rl_data[5]);
    	AT_SendString(UART2, trx);
    	sleep(10);
    	// AT_CheckDataRate(UART2);
    	// memset(trx, 0, MAX_PAYLOAD_LENGTH);
    }
    
    csv_fini(&p, cb1, cb2, NULL);
    fclose(fp);
    printf("Finished parsing\n");
    csv_free(&p);
    
    exit(EXIT_SUCCESS);
}
 
 
 
 
 
/*******************************************************************************
 *
 * TESTING
 * 
 ******************************************************************************/
#ifdef TESTING
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
	
#ifdef T_LORA
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
	

	
	//Commented out for same reason as SetNwkSKey
	if (ATModule_SetDataRate(UART2, 1)){
		printf("Error setting device data rate.\n");
	}
	
	if (ATModule_CheckDataRate(UART2)){
		printf("Error retrieving device data rate.\n");
	}
	ATModule_SerialTransmit(UART2, "AT+CH\n");
	rc_usleep(5000);
	ATModule_SerialTransmit(UART2, "AT+LW=LEN\n");
	rc_usleep(5000);
	uint8_t buf[MAX_PAYLOAD_LENGTH] = {0};
	ATModule_SerialReceive(UART2, buf);
	printf("%s\n", buf);
	// if (ATModule_LowPower(UART2, 0)){
	// 	printf("Error entering low-power mode.\n");
	// }
	
	
	char *str= "Dick and Balls";
	if (ATModule_SendString(UART2, str)){
		printf("Error sending string.\n");
	}
	
#endif

#ifdef T_CSV
		
	FILE *fp;
    struct csv_parser p;
    char buf[BUFFER_SIZE];
    size_t bytes_read;
	struct rl_samples rl = {0};

    if (csv_init(&p, 0) != 0){
        printf("init fail\n");
        exit(EXIT_FAILURE);
    } 
    fp = fopen("samples/rocketlogger.csv", "r");
    if (!fp){
        printf("fopen fail\n");
        exit (EXIT_FAILURE);
    } 
    csv_set_opts(&p, CSV_APPEND_NULL);
    while ((bytes_read=fread(buf, 1, 1024, fp)) > 0){
        if (csv_parse(&p, buf, bytes_read, cb1, cb2, &rl) != bytes_read) {
            fprintf(stderr, "Error while parsing file: %s\n",
            csv_strerror(csv_error(&p)) );
            exit(EXIT_FAILURE);
        }
    }
        csv_fini(&p, cb1, cb2, NULL);
        fclose(fp);
        printf("Finished parsing\n");
        csv_free(&p);
        printf("I1H Avg: %i\n", rl.rl_data[I1H-3]);
        printf("I1L Avg: %i\n", rl.rl_data[I1L-3]);
        printf("V1 Avg: %i\n", rl.rl_data[V1-3]);
        printf("V2 Avg: %i\n", rl.rl_data[V2-3]);
        printf("I2H Avg: %i\n", rl.rl_data[I2H-3]);
        printf("I1L Avg: %i\n", rl.rl_data[I1L-3]);
        
        
        exit(EXIT_SUCCESS);
#endif



	return 0;
}
#endif