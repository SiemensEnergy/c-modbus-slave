#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

extern int server_init(int port);
extern int server_poll(int ss, const int *cs, size_t ncss, int *is_new_conn);
extern ssize_t server_recv(int s, uint8_t *buf, size_t len);
extern ssize_t server_send(int s, const uint8_t *buf, size_t len);
extern int server_close(int s);

#endif /* SERVER_H_INCLUDED */
