#include "client.h"

int ipc_client_setup(int fd, const char * path)
{
	int rc;
	struct sockaddr_un addr;

	ipc_set_addr(&addr, path);

	rc = connect(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un));
	return rc;
}
