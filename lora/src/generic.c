#include "generic.h"

int ipc_open(void)
{
	// Open socket
	int fd;
	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		error(EXIT_FAILURE, errno, "Could not create socket");
	}

	return fd;
}

int ipc_write(int fd, const char * buf, const int n)
{
	int nw;
	nw = write(fd, buf, n);
	if (nw < 0)
		error(EXIT_FAILURE, errno, "Could not write");
	if (nw != n)
		error(0, 0, "Incomplete write");

	return nw;
}

int ipc_read(int fd, char * buf, int n)
{
	int nr;

	nr = read(fd, buf, n);

	if (nr < 0)
		error(EXIT_FAILURE, errno, "Could not read");

	return nr;
}

void ipc_set_addr(struct sockaddr_un * addr, const char * path) {
	if (strlen(path) > sizeof(addr->sun_path) - 1) {
		error(EXIT_FAILURE, 0, "Socket path too long");
	}

	// Settings
	memset(addr, 0, sizeof(struct sockaddr_un));
	addr->sun_family = AF_UNIX;
	strncpy(addr->sun_path, path, strlen(path));
}

void ipc_close(int fd){
	int rc;
	rc = close(fd);
	if (rc < 0)
		error(EXIT_FAILURE, errno, "Could not gracefully close the connection");
}
