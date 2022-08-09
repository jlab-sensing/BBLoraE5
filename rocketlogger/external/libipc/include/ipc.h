/**
 * @file ipc.h
 * @author John Madden (jtmadden@ucsc.edu)
 * @brief Main header file for IPC.
 * @date 2022-07-27
 */

#ifndef IPC_H
#define IPC_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a new server
 *
 * @param fname Socket filename
 * @return File descriptor of socket
 */
int ipc_server(const char * fname);

/**
 * @brief Accept an incomming connection
 *
 * On failure, errno is updated.
 *
 * @param fd Socket file descriptor
 * @return Client file descriptor, -1 indicates failure
 */
int ipc_server_accept(int fd);

/**
 * @brief Creates client and connects to socket
 *
 * @param fname Socket filename
 * @return Socket file descriptor
 */
int ipc_client(const char * fname);

/**
 * @brief Writes to connection
 *
 * Will continue to call write() until all characters in buf are written.On
 * failure, errno is updated.
 *
 * @param fd File descriptor of socket
 * @param buf Pointer to input character buffer
 * @param n Number of characters to write
 * @return Number of characters written, values of -1 indicates failure
 */
int ipc_write(int fd, const char * buf, const int n);

/**
 * @brief Reads from connection
 *
 * On failure, errno is updated.
 *
 * @param buf Pointer to store read characters
 * @param n Max number of characters to read
 * @return Number of characters read, values of -1 indicate failure
 */
int ipc_read(int fd, char * buf, int n);

/**
 * @brief Closes connection
 *
 * @param fd Socket file descriptor
 */
void ipc_close(int fd);

#ifdef __cplusplus
}
#endif

#endif /* IPC_H */
