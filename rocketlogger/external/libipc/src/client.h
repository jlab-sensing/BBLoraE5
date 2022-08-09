#ifndef IPC_CLIENT_H
#define IPC_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <error.h>
#include <unistd.h>

#include "generic.h"

/**
 * @brief Setup socket as client
 *
 * On failure, errno is updated.
 *
 * @param fd Socket file descriptor
 * @param path Path to socket file
 * @return Returns 1 on success, 0 on failure
 */
int ipc_client_setup(int fd, const char * path);

#ifdef __cplusplus
}
#endif

#endif /* IPC_CLIENT_H */
