#include "server.h"

void ipc_server_setup(int fd, char * path)
{
	int rc;
	struct sockaddr_un addr;

	// Remove socket if one exists
	rc = remove(path);
	if (rc < 0) {
		if (errno != ENOENT) {
			error(EXIT_FAILURE, errno, "Could not remove existing socket");
		}
	}

	ipc_set_addr(&addr, path);

	// Bind socket to address
	rc = bind(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un));
	if (rc < 0)
		error(EXIT_FAILURE, errno, "Could not bind socket");

	// Listen for connections
	rc = listen(fd, BACKLOG);
	if (rc < 0)
		error(EXIT_FAILURE, errno, "Could not start listening");
}

int ipc_server_accept(int fd)
{
	// Accept an incoming connection
	int cfg;
	
	cfg = accept(fd, NULL, NULL);
	if (cfg < 0)
		error(EXIT_FAILURE, errno, "Could not accept connection");

	return cfg;
}
