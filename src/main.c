/*******************************************************************************
 * Created by: Brian Govers
 * Name: main.c
 * Date of Creation: 27 July 2022
 ******************************************************************************/
 
 #include <stdio.h>
 #include <stdlib.h>
 #include "lora.h"
 #include "csv.h"
 
//  #define T_LORA
 #define T_CSV
 #define BUFFER_SIZE 1024
 #define NUM_SENSORS 2
 #define NUM_SAMPLES 499

 struct t_samples{
 	int raw_vwc[NUM_SENSORS][NUM_SAMPLES];
 	int temp[NUM_SENSORS][NUM_SAMPLES];
 	int ec[NUM_SENSORS][NUM_SAMPLES];
 };

static uint8_t col = 0;
 
void cb1 (void *s, size_t len, void *data){
	static int id = 0;
	uint8_t *chr = (uint8_t*)s;
	
	if (col==0){
		id = strtol((char*)s, NULL, 10);
		printf("%i\n", id);
	}
	col++;
}
void cb2 (int c, void *data){
	col = 0;
}
 
int main(void){
	printf("\nBegin testing on %s at %s\n\n", __DATE__, __TIME__);
	
#ifdef T_LORA
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
	
	if (ATModule_CheckDataRate(UART2)){
		printf("Error retrieving device data rate.\n");
	}
	
	//Commented out for same reason as SetNwkSKey
	// if (ATModule_SetDataRate(UART2, 0)){
	// 	printf("Error setting device data rate.\n");
	// }
	
	if (ATModule_LowPower(UART2, 0)){
		printf("Error entering low-power mode.\n");
	}
#endif

#ifdef T_CSV
		
	FILE *fp;
    struct csv_parser p;
    char buf[BUFFER_SIZE];
    size_t bytes_read;
    // const char teros = "sample/teros.csv";
    
    if (csv_init(&p, 0) != 0){
        printf("init fail\n");
        exit(EXIT_FAILURE);
    } 
    fp = fopen("samples/teros.csv", "r");
    if (!fp){
        printf("fopen fail\n");
        exit (EXIT_FAILURE);
    } 
    csv_set_opts(&p, CSV_APPEND_NULL);
    while ((bytes_read=fread(buf, 1, 1024, fp)) > 0){
        if (csv_parse(&p, buf, bytes_read, cb1, cb2, NULL) != bytes_read) {
            fprintf(stderr, "Error while parsing file: %s\n",
            csv_strerror(csv_error(&p)) );
            exit(EXIT_FAILURE);
        }
        // printf("%c", p.entry_buf[1]);
    }
        // csv_fini(&p, cb1, cb2, &c);
        fclose(fp);
        printf("Finished parsing\n");
        csv_free(&p);
        exit(EXIT_SUCCESS);
#endif

	return 0;
}