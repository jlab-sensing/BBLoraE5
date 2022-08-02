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
#include "ipc.h"
 
#define NUM_RL_FIELDS 6
#define NUM_SAMPLES 500
#define NUM_TSAMPLES 2
#define BUF_LEN 1024

/*******************************************************************************
 * 
 * MACRO DEFINITIONS
 * 
 ******************************************************************************/ 

#define PARSE_PREP col = 0; \
	num_samples = 1;

#define RL_IT_AVG(index, inp, samp) ((struct rl_samples*)data)->rl_data[index] +=  \
			(inp-((struct rl_samples*)data)->rl_data[index])/samp;
			
/*******************************************************************************
 * 
 * DATA TYPES/GLOBAL VARIABLES
 * 
 ******************************************************************************/ 
 
enum data_fields{TIMESTAMP = 0, I1LV, I2LV, I1H, I1L, V1, V2, I2H, I2L};

struct rl_samples{
 	uint8_t I1L_Valid, I2L_Valid;
	int rl_data[NUM_RL_FIELDS];
};

struct tsamples{
	int moisture;
	int temp;
	int rho;
};

static uint8_t col = 0;
static int num_samples = 1;

/*******************************************************************************
 * 
 * CALLBACK FUNCTIONS
 * 
 ******************************************************************************/ 
 
void cb1 (void *s, size_t len, void *data){
	int chr = 0;
	chr = strtol((char*)s, NULL, 10);
	
	//quick n easy, change later
	if (col == I1LV){ 
		((struct rl_samples*)data)->I1L_Valid = chr;
	} else if (col == I2LV){ 
		((struct rl_samples*)data)->I2L_Valid = chr;
	} else if (col == I1H){ 
		RL_IT_AVG(0, chr, num_samples);
	} else if (col == I1L){ 
		RL_IT_AVG(1, chr, num_samples);
	} else if (col == V1){ 
		RL_IT_AVG(2, chr, num_samples);
	} else if (col == V2){
		RL_IT_AVG(3, chr, num_samples);
	} else if (col == I2H){
		RL_IT_AVG(4, chr, num_samples);
	} else if (col == I2L){
		RL_IT_AVG(5, chr, num_samples);
	} 
	
	col++;
}
void cb2 (int c, void *data){
	col = 0;
	num_samples++;
}

void cb3 (void *s, size_t len, void *data){
	int chr = 0;
	chr = strtol((char*)s, NULL, 10);

	if (col == 0){
		((struct tsamples*)data)->moisture += \
			(chr-((struct tsamples*)data)->moisture)/num_samples;
	} else if (col == 1){
		((struct tsamples*)data)->temp += \
			(chr-((struct tsamples*)data)->temp)/num_samples;
	} else if (col == 2){
		((struct tsamples*)data)->rho += \
			(chr-((struct tsamples*)data)->rho)/num_samples;
	}
	col++;
}
 

 
/*******************************************************************************
 * 
 *	MAIN FUNCTION
 * 
 ******************************************************************************/ 


int main(void){
	printf("\nProgram compiled on %s at %s\n\n", __DATE__, __TIME__);

	if (AT_Init()){
		printf("Error initializing module\n");
		exit(EXIT_FAILURE);
	}
	
	//doesn't work inside of at_init for some reason
	if (AT_SetDataRate(UART2, 2) == -1){
		printf("Error setting datarate.\n");
	}
	
	//update socket with correct name for implementation
	int server = ipc_server("/tmp/libipc-example.socket");

	int cfd = ipc_server_accept(server);
	
	int num_read;
	size_t bytes_read=BUF_LEN;

	FILE *fp;
    struct csv_parser p;
	struct csv_parser p2;
    
	struct rl_samples rl = {0};
	struct tsamples ts = {0};
	
	char buf[BUF_LEN] = {0};
	char trx[MAX_PAYLOAD_LENGTH] = {0};


    if (csv_init(&p, 0) != 0){
        printf("csv init fail\n");
        exit(EXIT_FAILURE);
    } 
    if (csv_init(&p2, 0) != 0){
        printf("csv2 init fail\n");
        exit(EXIT_FAILURE);
    } 
	
	//update file path for proper implementation
    fp = fopen("samples/rocketlogger.csv", "r");
	
	if (!fp){
        printf("fopen fail\n");
        exit (EXIT_FAILURE);
    } 

    csv_set_opts(&p, CSV_APPEND_NULL);
    csv_set_opts(&p2, CSV_APPEND_NULL);

	//Get and process rocketlogger data
    PARSE_PREP;
    while((bytes_read=fread(&buf, 1, 1024, fp)) > 0){//was while
    	if (csv_parse(&p, buf, bytes_read, cb1, cb2, &rl) != bytes_read) {
            fprintf(stderr, "Error while parsing file: %s\n",
            csv_strerror(csv_error(&p)) );
            exit(EXIT_FAILURE);
        }
	}

    //Get and process teros data
    PARSE_PREP;
    while(num_samples<=NUM_TSAMPLES){
    	if ((num_read=ipc_read(cfd, buf, BUF_LEN)) > 0){
			csv_parse(&p2, buf, num_read, cb3, cb2, &ts);
			// printf("\n%i\t%i\t%i\n", ts.moisture, ts.temp, ts.rho);
		}
    }
    
	sprintf(trx, "%i,%i,%i,%i,%i,%i,%i,%i,%i", rl.rl_data[0], rl.rl_data[1], \
    	rl.rl_data[2], rl.rl_data[3], rl.rl_data[4], rl.rl_data[5],ts.moisture,\
    	ts.temp,ts.rho);
    	
    AT_SendString(UART2, trx);
	
    csv_fini(&p, cb1, cb2, NULL);
    csv_fini(&p2, cb3, NULL, NULL);
    fclose(fp);
    csv_free(&p);
    csv_free(&p2);
    
    exit(EXIT_SUCCESS);
}