/**
 * @file pipestream.c
 * @brief Reads from stdin and outputs to socket
 * @author John Madden (jtmadden@ucsc.edu)
 * @date 2022-08-03
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>

#include "ipc.h"
#include "csv.h"

#define BUF_LEN 1024

static char pstring[BUF_LEN];
static int col;
static int client;

void cb1(void *s, size_t len, void *data)
{
	strcat(pstring, (char *)(s));
	if (col < 8)
		strncat(pstring, ",", 1);
	col++;
}

void cb2(int c, void *data)
{
	col = 0;

	strncat(pstring, "\n", 1);
	int num_write = ipc_write(client, pstring, strlen(pstring));
	if (num_write < 0)
	{
		error(0, errno, "Could not write to socket");
	}

	int i = 0;
	while (pstring[i])
	{
		pstring[i] = '\0';
		i++;
	}
	// pstring[0] = '\n';
}

int main(int argc, char *argv[])
{
	printf("pipstream, compiled on %s %s\n", __DATE__, __TIME__);

	if (argc != 3)
	{
		error(EXIT_FAILURE, 0, "Missing socket file");
	}
	// argv[1] socket name
	// argv[2] file to read from

	char *socket_file = argv[1];

	// Connect to server socket

	struct csv_parser p;
	if (csv_init(&p, 0) != 0)
		exit(EXIT_FAILURE);
	csv_set_opts(&p, CSV_APPEND_NULL);

	FILE *fp = fopen(argv[2], "r");
	if (!fp)
		exit(EXIT_FAILURE);

	size_t bytes_read;

	for (;;)
	{
		client = ipc_client(socket_file);
		if (client < 0)
		{
			error(0, errno, "Could not open client socket");
			continue;
		}
		else
			printf("Opened RL client\n");

		// Read buffer
		char buf[BUF_LEN];
		col = 0;
		for (;;)
		{
			// Get from rl csv

			while ((bytes_read = fread(buf, 1, 1024, fp)) > 0)
			{
				if (csv_parse(&p, buf, bytes_read, cb1, cb2, &client) != bytes_read)
				{
					fprintf(stderr, "Error while parsing file: %s\n",
							csv_strerror(csv_error(&p)));
					exit(EXIT_FAILURE);
				}
			}
			ipc_close(client);
			return 0;
		}

		ipc_close(client);
	}

	ipc_close(client);

	return 0;
}
