/*******************************************************************************
 * Created by: Brian Govers
 * Name: main.c
 * Date of Creation: 27 July 2022
 ******************************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <errno.h>

#include "lora.h"
#include "ipc.h"

#define NUM_RL_FIELDS 6
#define NUM_SAMPLES 500
#define NUM_TSAMPLES 2
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

struct rl_samples{
	int I1L_Valid;
	int I2L_Valid;
	int rl_data[NUM_RL_FIELDS];
};

struct tsamples{
	int timestamp;
	float moisture;
	float temp;
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
	//rocketlogger has two fields
	uint f_valid[2];
	if (col!=TIMESTAMP_f) chr = strtol((char*)s, NULL, 10);

	//quick n easy, change later
	if (col == I1LV_f){ 
		((struct rl_samples*)data)->I1L_Valid = chr;
	} else if (col == I2LV_f){ 
		((struct rl_samples*)data)->I2L_Valid = chr;
	} 
	
	if (((struct rl_samples*)data)->I1L_Valid &&\ 
			((struct rl_samples*)data)->I2L_Valid){
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
	
	//We 
	if (col == 0){
		((struct tsamples*)data)->timestamp = chr;
	} else if (col == 2){
		((struct tsamples*)data)->moisture += \
			(chr-((struct tsamples*)data)->moisture)/num_samples;
	} else if (col == 3){
		((struct tsamples*)data)->temp += \
			(chr-((struct tsamples*)data)->temp)/num_samples;
	} else if (col == 4){
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

	// Create teros socket
	int t_server = ipc_server("/tmp/teros.socket");
	if (t_server < 0)
		error(EXIT_FAILURE, errno, "Could not create teros socket");

	// Create rl socket
	int rl_server = ipc_server("/tmp/rl.socket");
	if (rl_server < 0)
		error(EXIT_FAILURE, errno, "Could not create rl socket");

	// Create client file descriptors
	int cfd = ipc_server_accept(t_server);	
	int sfd = ipc_server_accept(rl_server);

	int num_read;
	size_t bytes_read=BUF_LEN;

	struct rl_samples rl = {0};
	struct tsamples ts = {0};
	
	char buf[BUF_LEN] = {0};
	char trx[MAX_PAYLOAD_LENGTH] = {0};


	//Get and process rocketlogger data
	PARSE_PREP;
	// while((bytes_read=fread(&buf, 1, 1024, fp)) > 0){//was while
	// while (num_samples <= NUM_TSAMPLES) {
	if ((num_read=ipc_read(sfd, buf, BUF_LEN)) > 0) {
	}

	//Get and process teros data
	PARSE_PREP;
	// while (num_samples <= NUM_TSAMPLES) {
	if ((num_read=ipc_read(cfd, buf, BUF_LEN)) > 0) {
	}
	
	sprintf(trx, "%i,%i,%i,%i,%i,%f,%f,%i", ts.timestamp, rl.rl_data[0], \
	rl.rl_data[2], rl.rl_data[3], rl.rl_data[5],ts.moisture, ts.temp, ts.rho);
		
	// printf("%s", trx);
	// char *tstring = "1615346218,-1251049,689212,25790567,32135,2646.57,19.8,219";
	AT_SendString(UART2, trx);

	return 0;
}
