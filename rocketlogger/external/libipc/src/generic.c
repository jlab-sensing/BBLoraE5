#include "generic.h"

int ipc_open(void)
{
	// Open socket
	int fd;
	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	return fd;
}

int ipc_write(int fd, const char * buf, const int n)
{
	int rc;
	int nw = 0;

	// write until everything in buffer is written
	while (nw < n) {
		rc = write(fd, buf+nw, n-nw);

		// check for errors
		if (rc < 0) {
			return rc;
		}
		else {
			nw += rc;
		}
	}

	return nw;
}

int ipc_read(int fd, char * buf, int n)
{
	int nr;
	nr = read(fd, buf, n);
	return nr;
}

int ipc_set_addr(struct sockaddr_un * addr, const char * path) {
	if (strlen(path) > sizeof(addr->sun_path)) {
		errno = ENAMETOOLONG;
		return 0;
	}

	// Settings
	memset(addr, 0, sizeof(struct sockaddr_un));
	addr->sun_family = AF_UNIX;
	strncpy(addr->sun_path, path, strlen(path));

	return 1;
}

void ipc_close(int fd){
	int rc;
	rc = close(fd);
	if (rc < 0)
		error(EXIT_FAILURE, errno, "Could not gracefully close the connection");
}
