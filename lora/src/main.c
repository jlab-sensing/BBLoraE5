/*******************************************************************************
 * Created by: Brian Govers
 * Name: main.c
 * Date of Creation: 27 July 2022
 ******************************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <error.h>
#include <errno.h>
#include "lora.h"
#include "csv.h"
#include "ipc.h"

#define NUM_RL_FIELDS 6
#define NUM_RL_SAMPLES 30
#define NUM_T_SAMPLES 3
#define BUF_LEN 1024

#define TIMESTAMP_LENGTH 22
#define END_OF_RL_HEADER 10 //actual rocketlogger data occurs on line 12 of csv

/*******************************************************************************
 * 
 * MACRO DEFINITIONS
 * 
 ******************************************************************************/ 

#define PARSE_PREP col = 0; \
	num_samples = 1; \

#define RL_IT_AVG(index, inp, samp) ((struct rl_samples*)data)->rl_data[index] +=  \
			(inp-((struct rl_samples*)data)->rl_data[index])/samp;

#define TS_IT_AVG(var, inp, samp) var += (inp - var)/samp;

#define RL_CONNECT 0
#define T_GATHER 1
#define RL_GATHER 2
#define TRANSMIT 3


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
static int num_samples = 0;
static uint8_t rflag = 0;

/*******************************************************************************
 * 
 * CALLBACK FUNCTIONS
 * 
 ******************************************************************************/ 
 
 
//Callback function 1 -- called after each individual field is processed by parser
void cb1 (void *s, size_t len, void *data){
	int chr = 0;
	if (col!=TIMESTAMP_f) chr = strtol((char*)s, NULL, 10);

	//num_samples remains zero until it reaches the end of the file header
	if (num_samples){
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
	}
	col++;
	
	// rflag marks the beginning of csv parsing so we can skip csv header info
	if (!rflag) rflag = 1;
}

//Callback function 2 -- called at the end of individual teros fields
void cb2 (void *s, size_t len, void *data){
	int chr = 0;
	chr = strtol((char*)s, NULL, 10);
	
	if (col == 0){
		((tsamples*)data)->timestamp = chr;
	} else if (col == 2){
		TS_IT_AVG(((tsamples*)data)->moisture, chr, num_samples);
	} else if (col == 3){
		TS_IT_AVG(((tsamples*)data)->temp, chr, num_samples);
	} else if (col == 4){
			TS_IT_AVG(((tsamples*)data)->rho, chr, num_samples);

	}
	col++;
}

//Callback function 3 -- called at the end of each row
void cb3 (int c, void *data){
	static int begin = 0;
	
	/*
	If we're reading from the rocketlogger, we use BEGIN as a dummy variable
	to keep track of the rows until we reach the end of the header, at which
	point we begin incrementing the number of samples
	*/
	if (rflag){
		if (begin >= END_OF_RL_HEADER){
			num_samples++;
		} else {
			begin++;
		}	
	} else {
		num_samples++;
	}
	
	col = 0;
	
}



 
/*******************************************************************************
 * 
 *	MAIN FUNCTION
 * 
 ******************************************************************************/ 


int main(int argc, char *argv[]){
	printf("\n%s compiled on %s at %s\n", argv[0], __DATE__, __TIME__);
	
	//argv[1] = teros socket name
	//argv[2] = rocketlogger socket name
	if (argc != 3){
		error(EXIT_FAILURE, 0, "Missing program argument");
	}

	if (AT_Init()){
		printf("Error initializing module\n");
		exit(EXIT_FAILURE);
	}
	
	//create server and accept connection from teros socket
	int t_server = ipc_server(argv[1]);
	int t_fd = ipc_server_accept(t_server);

	//create server for rocketlogger
	int rl_server = ipc_server(argv[2]);

	struct rl_samples rl = {0};
	struct tsamples ts = {0};
	
	char buf[BUF_LEN] = {0};
	char trx[MAX_PAYLOAD_LENGTH] = {0};
	
	struct csv_parser p;
	struct csv_parser p2;

	//initialize parser structs for csv reading
    if (csv_init(&p, 0) != 0){
        printf("csv init fail\n");
        exit(EXIT_FAILURE);
    } 
    if (csv_init(&p2, 0) != 0){
        printf("csv2 init fail\n"); 
        exit(EXIT_FAILURE);
    } 
	
	//set option to append a newline to the end of each row
    csv_set_opts(&p, CSV_APPEND_NULL);
    csv_set_opts(&p2, CSV_APPEND_NULL);
    
	int rl_fd = 0;
	int bytes_read = 0;
	uint8_t state = RL_CONNECT;
	
    while(1)
    {
    	if (!rl_fd){
    		rl_fd = ipc_server_accept(rl_server);
			state = T_GATHER;
    	} else {
    		//Get and process teros data
    		/*
    		NOTE: It's important that we gather teros data before rl data.
    		If we gather rl data first, there won't be enough time for the
    		information to show up in the socket. ipc_read will return
    		EAGAIN (nothing to read), and it'll close the socket without
    		having read anything. Gathering teros data first ensures that the 
    		auxilliary program has enough time to send rocketlogger data before
    		we attempt to read from the socket. If you want to gather rl data
    		first, use rflag to stay in keep the socket open long enough to read
    		something.
    		*/
    		PARSE_PREP;
    		//arbitrary number of samples, find diff method (or better number)
    		while (num_samples<=100){ 
    		while ((bytes_read=ipc_read(t_fd, buf, BUF_LEN)) > 0) {
				if (csv_parse(&p2, buf, bytes_read, cb2, cb3, &ts) != bytes_read) {
					fprintf(stderr, "Error while parsing file: %s\n",
       				csv_strerror(csv_error(&p2)) );
       	 			exit(EXIT_FAILURE);
				}
    		}
    		}
    		
    		//Get and process rocketlogger data
    		PARSE_PREP;
    		while ((bytes_read=ipc_read(rl_fd, buf, BUF_LEN)) > 0) {
    			if (csv_parse(&p, buf, bytes_read, cb1, cb3, &rl) != bytes_read) {
	    			fprintf(stderr, "Error while parsing file: %s\n",
          			csv_strerror(csv_error(&p)) );
           			exit(EXIT_FAILURE);
       			}
    		}
    		
			// sprintf(trx, "%i,%i,%i,%i,%i,%f,%f,%i", ts.timestamp, rl.rl_data[0], \
				// rl.rl_data[2], rl.rl_data[3], rl.rl_data[5],ts.moisture, ts.temp, ts.rho);
			printf("String: %i,%i,%i,%i,%i,%f,%f,%i\n", ts.timestamp, rl.rl_data[0], \
				rl.rl_data[2], rl.rl_data[3], rl.rl_data[5],ts.moisture, ts.temp, ts.rho);
			// AT_SendString(UART2, trx);
			
			ipc_close(rl_fd);
    		rl_fd = 0;
    	}
	}
	
    csv_fini(&p, cb1, cb2, NULL);
    csv_fini(&p2, cb3, NULL, NULL);
    csv_free(&p);
    csv_free(&p2);
    
    exit(EXIT_SUCCESS);
}
