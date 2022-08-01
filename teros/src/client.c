#include "client.h"

void ipc_client_setup(int fd, const char * path)
{
	struct sockaddr_un addr;

	ipc_set_addr(&addr, path);

	int rc;
	rc = connect(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un));
	if (rc < 0)
		error(EXIT_FAILURE, errno, "Could not connect to server");
}
