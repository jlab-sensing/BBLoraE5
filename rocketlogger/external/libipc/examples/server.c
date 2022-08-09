/**
 * @file example_server.c
 * @brief Example server to read messages over Unix Domain Sockets.
 * @author John Madden (jtmadden@ucsc.edu)
 * @date 2022-08-1
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>

#include "ipc.h"

#define BUF_LEN 1024

int main(int argc, char * argv[]) {
	printf("Example server with libipc, compiled on %s %s\n", __DATE__, __TIME__);

	if (argc != 2) {
		printf("Missing socketfile!\n");
		exit(EXIT_FAILURE);
	}

	// Setup server scoket
	int server = ipc_server(argv[1]);
	if (server < 0)
		error(EXIT_FAILURE, errno, "Could not create socket server");

	while (1)
	{
		// Accept a connection
		int cfd = ipc_server_accept(server);
		if (cfd < 0)
			error(EXIT_FAILURE, errno, "Could not accept client connection");

		char buf[BUF_LEN];
		int num_read;

		// Continuously read from socket
		while (1) {
			num_read = ipc_read(cfd, buf, BUF_LEN);
			if (num_read < 0) {
				if (errno == EAGAIN)
					continue;
				else
					break;
			}

			// print
			fputs(buf, stdout);

			// Clear buffer
			memset(buf, 0, BUF_LEN);
		}
		
		// Handle errors
		if (num_read < 0)
			error(0, errno, "Could not read from socket");
	}

	// Close connection
	ipc_close(server);

	return 0;
}
