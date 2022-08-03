/**
 * @file generic.h
 * @author John Madden (jtmadden@ucsc.edu)
 * @brief Generic functions for handling Unix sockets.
 * @date 2022-07-27
 */

#ifndef GENERIC_H__
#define GENERIC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <error.h>
#include <unistd.h>

#include "ipc.h"

/**
 * @brief Opens a new socket
 *
 * @return File descriptor of socket
 */
int ipc_open(void);

/**
 * @brief Sets the socket address to file
 *
 * @param addr Pointer to socket address to configure
 * @param path Path to socket
 */
void ipc_set_addr(struct sockaddr_un * addr, const char * path);

#ifdef __cplusplus
}
#endif

#endif /* GENERIC_H__ */
