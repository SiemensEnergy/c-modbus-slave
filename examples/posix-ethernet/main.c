#include "modbus.h"
#include "server.h"

#include <mbadu_tcp.h>

#include <stdint.h>
#include <stdlib.h>

enum {MAX_NUM_CONNS=4};

int main(void)
{
	int ss, s;
	int is_new_conn;

	int cs[MAX_NUM_CONNS];
	size_t ncss = 0u;

	uint8_t rxbuf[MBADU_TCP_SIZE_MAX], txbuf[MBADU_TCP_SIZE_MAX];
	ssize_t nrxbuf, ntxbuf;

	ss = server_init(MBTCP_PORT);
	if (ss<0) {
		exit(EXIT_FAILURE);
	}

	modbus_init();

	while (1) {
		s = server_poll(ss, cs, ncss, &is_new_conn);

		if (is_new_conn) {
			if (ncss<MAX_NUM_CONNS-1) {
				cs[ncss++] = s;
			} else {
				server_close(s);
			}
		} else if (s>0) {
			nrxbuf = server_recv(s, rxbuf, sizeof rxbuf);

			if (nrxbuf>0) {
				if ((ntxbuf=mbadu_tcp_handle_req(modbus_get(), rxbuf, (size_t)nrxbuf, txbuf))>0) {
					server_send(s, txbuf, ntxbuf);
				} else {
					/* TODO: Remove from list, and defragment list */
					server_close(s);
				}
			} else {
				/* TODO: Remove from list, and defragment list */
				server_close(s);
			}
		}
	}

	return EXIT_SUCCESS;
}
