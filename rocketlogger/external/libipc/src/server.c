#include "server.h"

int ipc_server_setup(int fd, char * path)
{
	int rc;
	struct sockaddr_un addr;

	// Remove socket if one exists
	rc = remove(path);
	if (rc < 0 && errno != ENOENT)
		return 0;

	rc = ipc_set_addr(&addr, path);
	if (rc < 0)
		return 0;

	// Bind socket to address
	rc = bind(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un));
	if (rc < 0)
		return 0;

	// Listen for connections
	rc = listen(fd, BACKLOG);
	if (rc < 0)
		return 0;

	return 1;
}

int ipc_server_accept(int fd)
{
	int cfd;
	cfd = accept(fd, NULL, NULL);
	if (cfd < 0)
		return cfd;

	// get current flags
	int flags;
	flags = fcntl(cfd, F_GETFL);
	
	// set nonblocking
	int rc;
	rc = fcntl(cfd, F_SETFL, flags | O_NONBLOCK);
	if (rc < 0)
		return rc;

	return cfd;
}
