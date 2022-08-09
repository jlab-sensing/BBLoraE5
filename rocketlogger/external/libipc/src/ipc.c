#include "ipc.h"

#include <sys/socket.h>
#include <errno.h>
#include <error.h>

#include "generic.h"
#include "server.h"
#include "client.h"

int ipc_server(const char * fname)
{
	int fd;
	fd = ipc_open();
	ipc_server_setup(fd, fname);

	return fd;
}


int ipc_client(const char * fname)
{
	int fd;
	fd = ipc_open();
	ipc_client_setup(fd, fname);

	return fd;
}
