#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ipc.h"

#define BUF_LEN 1024

int main(int argc, char * argv[]) {
	printf("Teros client with libipc, compiled on %s %s\n", __DATE__, __TIME__);

	int client = ipc_client("/tmp/rlstream.socket");

	char buf[BUF_LEN];
	int num_read;
	while (1)
	{
		if(fgets(buf, BUF_LEN, stdin)!=NULL){
			num_read = ipc_write(client, buf, strlen(buf));
			sleep(1);
			// num_read = ipc_read(client, buf, BUF_LEN);
			// fputs(buf, stdout);
		}
	}

	ipc_close(client);

	return 0;
}