/**
 * @file server.h
 * @author John Madden (jtmadden@ucsc.edu)
 * @brief Abstraction layer for Unix socket server functions.
 * @date 2022-07-27
 */

#ifndef SERVER_H__
#define SERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "ipc.h"
#include "generic.h"

/** Backlog for connections */
#define BACKLOG 5

/**
 * @brief Setup socket as server
 *
 * Checks for exceeded path length and if the socket already exists. On
 * failure, errno is updated.
 *
 * @param fd Socket file descriptor
 * @param path Path to socket file
 * @return Returns 1 on success, 0 on failure
 */
int ipc_server_setup(int fd, char * path);

#ifdef __cplusplus
}
#endif

#endif /* SERVER_H__ */
