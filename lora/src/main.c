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
#define NUM_RL_SAMPLES 30
#define NUM_T_SAMPLES 3
#define BUF_LEN 1024

#define TIMESTAMP_LENGTH 22

/*******************************************************************************
 * 
 * MACRO DEFINITIONS
 * 
 ******************************************************************************/ 

#define PARSE_PREP col = 0; \
	num_samples = 1;

#define RL_IT_AVG(index, inp, samp) ((struct rl_samples*)data)->rl_data[index] +=  \
			(inp-((struct rl_samples*)data)->rl_data[index])/samp;

#define TS_IT_AVG(var, inp, samp) var += (inp - var)/samp;

/*******************************************************************************
 * 
 * DATA TYPES/GLOBAL VARIABLES
 * 
 ******************************************************************************/ 
 
enum rl_fields{
	TIMESTAMP_f = 0, 
	I1LV_f,
	I2LV_f,
	I1H_f,
	I1L_f,
	V1_f,
	V2_f,
	I2H_f,
	I2L_f
};

typedef struct rl_samples{
	int I1L_Valid;
	int I2L_Valid;
	int rl_data[NUM_RL_FIELDS];
} rl_samples;

typedef struct tsamples{
	int timestamp;
	float moisture;
	float temp;
	int rho;
} tsamples;

static uint8_t col = 0;
static int num_samples = 1;

/*******************************************************************************
 * 
 * CALLBACK FUNCTIONS
 * 
 ******************************************************************************/ 
 
void cb1 (void *s, size_t len, void *data){
	int chr = 0;
	//rocketlogger has two fields
	if (col!=TIMESTAMP_f) chr = strtol((char*)s, NULL, 10);

	//quick n easy, change later
	if (col == I1LV_f){ 
		((rl_samples*)data)->I1L_Valid = chr;
	} else if (col == I2LV_f){ 
		((rl_samples*)data)->I2L_Valid = chr;
	} 
	
	if (col == V1_f){ 
		RL_IT_AVG(0, chr, num_samples);
	} else if (col == I1H_f){ 
		RL_IT_AVG(1, chr, num_samples);
	} else if (col == I1L_f){ 
		RL_IT_AVG(2, chr, num_samples);
	} else if (col == V2_f){
		RL_IT_AVG(3, chr, num_samples);
	} else if (col == I2H_f){
		RL_IT_AVG(4, chr, num_samples);
	} else if (col == I2L_f){
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
		((struct tsamples*)data)->timestamp = chr;
	} else if (col == 2){
		TS_IT_AVG(((tsamples*)data)->moisture, chr, num_samples);
	} else if (col == 3){
		TS_IT_AVG(((tsamples*)data)->temp, chr, num_samples);
	} else if (col == 4){
			TS_IT_AVG(((tsamples*)data)->rho, chr, num_samples);

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
	
	//update socket with correct name for implementation
	int t_server = ipc_server("/tmp/terosstream.socket");
	int cfd = ipc_server_accept(t_server);
	
	int rl_server = ipc_server("/tmp/rlstream.socket");
	int sfd = ipc_server_accept(rl_server);

	// int num_read;
	int bytes_read;

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
	

    csv_set_opts(&p, CSV_APPEND_NULL);
    csv_set_opts(&p2, CSV_APPEND_NULL);

	//Get and process rocketlogger data
	while(1){
		
    PARSE_PREP;
    while (num_samples <= NUM_RL_SAMPLES) {
    	if ((bytes_read=ipc_read(sfd, buf, BUF_LEN)) > 0) {
    		if (csv_parse(&p, buf, bytes_read, cb1, cb2, &rl) != bytes_read) {
            	fprintf(stderr, "Error while parsing file: %s\n",
            	csv_strerror(csv_error(&p)) );
            	exit(EXIT_FAILURE);
        	}
		}
    }

    //Get and process teros data
    PARSE_PREP;
    while (num_samples <= NUM_T_SAMPLES) {
    	if ((bytes_read=ipc_read(cfd, buf, BUF_LEN)) > 0) {
			if (csv_parse(&p2, buf, bytes_read, cb3, cb2, &ts) != bytes_read) {
				fprintf(stderr, "Error while parsing file: %s\n",
            	csv_strerror(csv_error(&p2)) );
            	exit(EXIT_FAILURE);
			}
    	}
    }
    
	sprintf(trx, "%i,%i,%i,%i,%i,%f,%f,%i", ts.timestamp, rl.rl_data[0], \
		rl.rl_data[2], rl.rl_data[3], rl.rl_data[5],ts.moisture, ts.temp, ts.rho);
    	
    AT_SendString(UART2, trx);
    }
	
    csv_fini(&p, cb1, cb2, NULL);
    csv_fini(&p2, cb3, NULL, NULL);
    csv_free(&p);
    csv_free(&p2);
    
    exit(EXIT_SUCCESS);
}