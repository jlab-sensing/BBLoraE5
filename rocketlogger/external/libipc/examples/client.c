/**
 * @file example_client.c
 * @brief Example client to send stdin over Unix Domain Sockets.
 * @author John Madden (jtmadden@ucsc.edu)
 * @date 2022-08-01
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>

#include "ipc.h"

#define BUF_LEN 1024

int main(int argc, char * argv[]) {
	printf("Example client with libipc, compiled on %s %s\n", __DATE__, __TIME__);

	if (argc != 2) {
		printf("Missing socketfile!\n");
		exit(EXIT_FAILURE);
	}

	// Connect to server socket
	int client = ipc_client(argv[1]);
	if (client < 0)
		error(EXIT_FAILURE, errno, "Could not open client socket");

	char buf[BUF_LEN];
	int num_write;
	int num_read;
	while (1)
	{
		fgets(buf, BUF_LEN, stdin);

		num_write = ipc_write(client, buf, strlen(buf));
		if (num_write < 0)
			error(EXIT_FAILURE, errno, "Could not write to socket");
	}

	ipc_close(client);

	return 0;
}
