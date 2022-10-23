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
#include <math.h>
#include "lora.h"
#include "csv.h"
#include "ipc.h"

/*******************************************************************************
 *
 * MACRO DEFINITIONS
 *
 ******************************************************************************/

#define VOLTAGE 0
#define CURRENT 1

#define LORA 0
#define ETHERNET 1

#define BUF_LEN 1024
#define NAME_BUF 32

#define T_HEADER_LENGTH 1
#define RL_HEADER_LENGTH 10

#define PARSE_PREP \
	col = 0;       \
	num_samples = 1;

#define ITERATIVE_AVG(var, inp, samp) var += (inp - var) / samp;

#define RL_CONNECT 0
#define T_GATHER 1
#define RL_GATHER 2
#define TRANSMIT 3

/*******************************************************************************
 *
 * DATA TYPES/GLOBAL VARIABLES
 *
 ******************************************************************************/

typedef enum rl_fields
{
	RL_TIMESTAMP = 0,
	I1LV,
	I2LV,
	I1H,
	I1L,
	V1,
	V2,
	I2H,
	I2L
} rl_fields;

typedef enum teros_fields
{
	T_TIMESTAMP = 0,
	SENSOR_ID,
	MOISTURE,
	TEMP,
	CONDUCTIVITY
} teros_fields;

typedef struct sensor_data
{
	int timestamp;
	int rl_channel_1[2];
	int rl_channel_2[2];
	double moisture;
	double temp;
	int conductivity;
} sensor_data;

typedef struct config_info
{
	char cellNames[1024];
	char *username;
	uint8_t method;
	int num_Coeffs;
	double coeff[1024];
} config_info;

static int num_rl_rows = 0;
static int num_t_rows = 0;
static int num_samples = 0;
static rl_fields rl_col = RL_TIMESTAMP;
static teros_fields t_col = T_TIMESTAMP;

/*******************************************************************************
 *
 * CALLBACK FUNCTIONS
 *
 ******************************************************************************/

static void cb1(void *s, size_t len, void *data);
static void cb2(int c, void *data);
static void cb3(void *s, size_t len, void *data);
static void cb4(int c, void *data);
static void read_config(struct config_info *id);
static double raw_to_vwc(struct config_info *id, double raw_data);

/*******************************************************************************
 *
 *	MAIN FUNCTION
 *
 ******************************************************************************/

int main(int argc, char *argv[])
{
	printf("\n%s compiled on %s at %s\n", argv[0], __DATE__, __TIME__);

	// argv[1] = teros socket name
	// argv[2] = rocketlogger socket name
	// argv[3] = number of rocketlogger samples

	// check for valid number of cli arguments
	if (argc != 4)
	{
		error(EXIT_FAILURE, 0, "Missing program argument");
	}

	// ensure number of rocketlogger samples to take is valid
	int min_rl_samples = strtol(argv[3], NULL, 10);
	if (min_rl_samples <= 0)
	{
		error(EXIT_FAILURE, 0, "Invalid number of rocketlogger samples");
	}

	static config_info cellInfo = {0};

	cellInfo.username = getenv("LOGNAME");
	read_config(&cellInfo);
	char tmsg[BUF_LEN] = {0};

	if (cellInfo.method == LORA)
	{
		// initialize UART bus for lora
		if (AT_Init(UART5, BAUD96, TIMEOUT))
		{
			error(EXIT_FAILURE, 0, "Error initializing LoRaWAN module");
		}
	}

	// create server for rocketlogger
	int rl_server = ipc_server(argv[2]);
	printf("Rocketlogger server created\n");

	// create server and accept connection from teros socket
	int t_server = ipc_server(argv[1]);
	printf("Teros server created\n");

	int t_fd = ipc_server_accept(t_server);
	printf("Teros client accepted\n");

	char buf[BUF_LEN] = {0};
	char lora_msg[MAX_PAYLOAD_LENGTH] = {0};

	sensor_data soil_data = {0};

	struct csv_parser p;
	struct csv_parser p2;

	// initialize parser structs for csv reading
	if (csv_init(&p, 0) != 0)
	{
		printf("csv init fail\n");
		exit(EXIT_FAILURE);
	}
	if (csv_init(&p2, 0) != 0)
	{
		printf("csv2 init fail\n");
		exit(EXIT_FAILURE);
	}

	// set option to append a newline to the end of each row
	csv_set_opts(&p, CSV_APPEND_NULL);
	csv_set_opts(&p2, CSV_APPEND_NULL);

	int rl_fd = 0;
	int t_bytes_read = 0;
	int rl_bytes_read = 0;
	num_samples = 0;

	while (1)
	{

		if (rl_fd <= 0)
		{
			// wait until rocketlogger successfully connects to the socket
			rl_fd = ipc_server_accept(rl_server);
			printf("RL client accepted\n");
		}
		else
		{
			// collect and send data samples
			if ((t_bytes_read = ipc_read(t_fd, buf, BUF_LEN)) > 0)
			{
				if (csv_parse(&p2, buf, t_bytes_read, cb3, cb4, &soil_data) != t_bytes_read)
				{
					fprintf(stderr, "Error while parsing file: %s\n",
							csv_strerror(csv_error(&p2)));
					exit(EXIT_FAILURE);
				}
			}

			if ((rl_bytes_read = ipc_read(rl_fd, buf, BUF_LEN)) > 0)
			{
				if (csv_parse(&p, buf, rl_bytes_read, cb1, cb2, &soil_data) != rl_bytes_read)
				{
					fprintf(stderr, "Error while parsing file: %s\n",
							csv_strerror(csv_error(&p)));
					exit(EXIT_FAILURE);
				}
			}

			if (num_samples >= min_rl_samples && num_t_rows >= 1)
			{
				soil_data.moisture = raw_to_vwc(&cellInfo, soil_data.moisture);
				sprintf(lora_msg, "%i,%i,%i,%i,%i,%f,%f,%i", soil_data.timestamp,
						soil_data.rl_channel_1[VOLTAGE], soil_data.rl_channel_1[CURRENT],
						soil_data.rl_channel_2[VOLTAGE], soil_data.rl_channel_2[CURRENT],
						soil_data.moisture, soil_data.temp, soil_data.conductivity);

				printf("Payload: %s\n", lora_msg);

				if (cellInfo.method == LORA)
				{
					// Send data samples to LoRaWAN gateway
					if (AT_SendString(UART5, lora_msg))
					{
						error(EXIT_FAILURE, 0, "Error sending LoRa packet");
					}
				}
				else if (cellInfo.method == ETHERNET)
				{
					memset(tmsg, '\0', BUF_LEN);
					// Send POST request to jlab server
					sprintf(tmsg, "curl -X POST -H \"Content-Type: mfc-data\""
								  " -H \"Cells: %s\" -H \"Device-Name: %s\""
								  " -d \"%s\" jlab.ucsc.edu:8090",
							cellInfo.cellNames, cellInfo.username, lora_msg);
					printf("\n%s\n", tmsg);
					system(tmsg);
				}
				else
				{
					error(EXIT_FAILURE, 0, "Invalid transmission method");
				}

				ipc_close(rl_fd);
				rl_fd = 0;
				memset(lora_msg, '\0', MAX_PAYLOAD_LENGTH);

				// clear all averaged fields to obtain new values in next loop
				soil_data.rl_channel_1[VOLTAGE] = 0;
				soil_data.rl_channel_1[CURRENT] = 0;
				soil_data.rl_channel_2[VOLTAGE] = 0;
				soil_data.rl_channel_2[CURRENT] = 0;
				soil_data.moisture = 0;
				soil_data.temp = 0;
				soil_data.conductivity = 0;

				// reset row and sample count
				num_t_rows = 0;
				num_rl_rows = 0;
				num_samples = 0;
				rl_col = RL_TIMESTAMP;
				t_col = T_TIMESTAMP;
			}
		}
	}

	csv_fini(&p, cb1, cb2, NULL);
	csv_fini(&p2, cb3, cb4, NULL);
	csv_free(&p);
	csv_free(&p2);

	exit(EXIT_SUCCESS);
}

/*******************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 ******************************************************************************/

// Callback function 1 -- called after each individual field is processed by parser
static void cb1(void *s, size_t len, void *data)
{
	int chr = 0;

	chr = strtol((char *)s, NULL, 10);

	// num_samples remains zero until it reaches the end of the file header
	if (num_rl_rows >= RL_HEADER_LENGTH)
	{
		/* The total number of samples is increased when parsing the header
		 * because each header line triggers cb2, so we subtract the line length
		 * of the header to get the correct number of valid samples*/
		num_samples = num_rl_rows - (RL_HEADER_LENGTH - 1);
		if (rl_col == RL_TIMESTAMP)
		{
			((sensor_data *)data)->timestamp = chr;
		}
		else if (rl_col == V1)
		{
			ITERATIVE_AVG(((sensor_data *)data)->rl_channel_1[VOLTAGE], chr, num_samples);
		}
		else if (rl_col == I1L)
		{
			ITERATIVE_AVG(((sensor_data *)data)->rl_channel_1[CURRENT], chr, num_samples);
		}
		else if (rl_col == V2)
		{
			ITERATIVE_AVG(((sensor_data *)data)->rl_channel_2[VOLTAGE], chr, num_samples);
		}
		else if (rl_col == I2L)
		{
			ITERATIVE_AVG(((sensor_data *)data)->rl_channel_2[CURRENT], chr, num_samples);
		}
	}
	rl_col++;
}

// Callback function 2 -- called at the end of each rocketlogger row
static void cb2(int c, void *data)
{
	num_rl_rows++;
	rl_col = RL_TIMESTAMP; // reset column index to 0
}

// Callback function 3 -- called at the end of individual teros fields
static void cb3(void *s, size_t len, void *data)
{
	double chr = 0;
	if (num_t_rows >= T_HEADER_LENGTH)
	{
		if (t_col == MOISTURE)
		{
			chr = strtof((char *)s, NULL);
			ITERATIVE_AVG(((sensor_data *)data)->moisture, chr, num_t_rows);
		}
		else if (t_col == TEMP)
		{
			chr = strtof((char *)s, NULL);
			ITERATIVE_AVG(((sensor_data *)data)->temp, chr, num_t_rows);
		}
		else if (t_col == CONDUCTIVITY)
		{
			chr = strtol((char *)s, NULL, 10);
			ITERATIVE_AVG(((sensor_data *)data)->conductivity, chr, num_t_rows);
		}
	}

	t_col++;
}

// Callback function 4 -- called at the end of each teros row
static void cb4(int c, void *data)
{
	num_t_rows++;
	t_col = T_TIMESTAMP; // reset column index to 0
}

// Obtains config info -- cellnames, tx method, calibration coefficients
static void read_config(struct config_info *id)
{
	FILE *fp;
	fp = fopen("rl.conf", "r");
	char buf[1024];

	if (fp == NULL)
	{
		error(EXIT_FAILURE, 0, "Unable to open config file");
	}

	// get tx method
	if (fgets(buf, 1024, fp) == NULL)
	{
		error(EXIT_FAILURE, 0, "Unable to retrieve transmission method");
	}
	buf[strcspn(buf, "\n")] = 0; // remove newline character

	if (!strcmp(buf, "lora"))
	{
		id->method = 0;
	}
	else if (!strcmp(buf, "ethernet"))
	{
		id->method = 1;
	}
	else
	{
		error(EXIT_FAILURE, 0, "Invalid data transmission method");
	}

	// get cells belonging to logger
	if (fgets(id->cellNames, 1024, fp) == NULL)
	{
		error(EXIT_FAILURE, 0, "Unable to retrieve cell names");
	}
	id->cellNames[strcspn(id->cellNames, "\n")] = 0; // remove newline character

	// get calibration coefficients
	if (fgets(buf, 1024, fp) == NULL)
	{
		error(EXIT_FAILURE, 0, "Unable to retrieve cell names");
	}
	char *pend, *check;
	/*strtof returns 0 if the conversion fails, which makes it impossible to
	determine if the coefficient is 0, or if it failed. Since it updates our
	second argument to the first char after the number, we can check to see
	if the memory location has changed since the last call.*/
	id->coeff[id->num_Coeffs] = strtof(buf, &pend);
	id->num_Coeffs++;
	do
	{
		check = pend;
		id->coeff[id->num_Coeffs] = strtof(pend, &pend);
		id->num_Coeffs++;

	} while (pend != check);
	/*it'll add an extra number to num_Coeffs since it updates before we check
	for failure, so take 1 off when it leaves.*/
	id->num_Coeffs--;

	fclose(fp);
}

static double raw_to_vwc(struct config_info *id, double raw_data)
{
	double retVal = 0;
	int i = 0;

	for (i = 0; i < id->num_Coeffs; i++)
	{
		retVal += id->coeff[i] * pow(raw_data, (double)i);
	}
	return retVal;
}