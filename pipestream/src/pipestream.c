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

#define BUF_LEN 1024

int main(int argc, char * argv[]) {
	printf("pipstream, compiled on %s %s\n", __DATE__, __TIME__);

	if (argc != 2) {
		error(EXIT_FAILURE, 0, "Missing socket file");
	}

	char * socket_file = argv[1];

	int client;
	for (;;)
	{
		// Connect to server socket
		client = ipc_client(socket_file);
		if (client < 0) {
			error(0, errno, "Could not open client socket");
			continue;
		}

		// Read buffer
		char buf[BUF_LEN];

		for (;;)
		{
			// Get from stdin
			fgets(buf, BUF_LEN, stdin);

			// Write to the socket
			int num_write = ipc_write(client, buf, strlen(buf));
			if (num_write < 0) {
				error(0, errno, "Could not write to socket");
				break;
			}
		}

		ipc_close(client);
	}

	ipc_close(client);

	return 0;
}
