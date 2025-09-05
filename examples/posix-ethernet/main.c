#include "modbus.h"
#include "server.h"

#include <mbadu_tcp.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

enum {MAX_NUM_CONNS=4};

void fatal(const char *fmt, ...)
{
	va_list args;

	fprintf(stderr, "Error: ");

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	exit(EXIT_FAILURE);
}

void usage(const char *cmd)
{
	fprintf(stderr, "Usage: %s [OPTIONS]\n", cmd);
	fprintf(stderr, "OPTIONS:\n");
	fprintf(stderr, " -h              Print this help message and exit\n");
	fprintf(stderr, " -p <port>       Use <port> as server port\n");
	fprintf(stderr, " -s              Do not print action logs\n");
}

int main(int argc, char *argv[])
{
	(void)argc;

	const char *cmd = *argv;

	int port = MBTCP_PORT;
	int silent = 0;

	int ss, s;
	int is_new_conn;

	int cs[MAX_NUM_CONNS] = {0};
	size_t ncs;

	uint8_t rxbuf[MBADU_TCP_SIZE_MAX], txbuf[MBADU_TCP_SIZE_MAX];
	ssize_t nrxbuf, ntxbuf;

	while (*++argv) {
		if (!strcmp(*argv, "-h")) {
			usage(cmd);
			exit(EXIT_SUCCESS);
		} else if (!strcmp(*argv, "-p")) {
			if (!*++argv) {
				usage(cmd);
				fatal("Option -p must be followed by a number");
			}
			port = atoi(*argv);
		} else if (!strcmp(*argv, "-s")) {
			silent = 1;
		} else {
			usage(cmd);
			fatal("Unknown option %s", *argv);
		}
	}

	if (!silent) printf("Starting server on port %d.\n", port);
	ss = server_init(port);
	if (ss<0) {
		fatal("Failed starting server on port %d", port);
	}

	modbus_init();

	while (1) {
		s = server_poll(ss, cs, MAX_NUM_CONNS, &is_new_conn);

		if (is_new_conn) {
			for (ncs = 0; ncs<MAX_NUM_CONNS; ++ncs) {
				if (!cs[ncs]) {
					cs[ncs] = s;
					if (!silent) printf("New connection.\n");
					break;
				}
			}
			if (ncs>=MAX_NUM_CONNS) {
				server_close(s);
				if (!silent) printf("New connection rejected. Maximum connection (%d) reached.\n", MAX_NUM_CONNS);
			}
		} else if (s>0) {
			nrxbuf = server_recv(s, rxbuf, sizeof rxbuf);

			if (nrxbuf>0) {
				if ((ntxbuf=mbadu_tcp_handle_req(modbus_get(), rxbuf, (size_t)nrxbuf, txbuf))>0) {
					(void)server_send(s, txbuf, ntxbuf);
				} else {
					server_close(s);
					for (ncs = 0; ncs<MAX_NUM_CONNS; ++ncs) {
						if (cs[ncs]==s) {
							cs[ncs] = 0;
							break;
						}
					}
					if (!silent) printf("Malformed packet received. Closing connection.\n");
				}
			} else {
				server_close(s);
				for (ncs = 0; ncs<MAX_NUM_CONNS; ++ncs) {
					if (cs[ncs]==s) {
						cs[ncs] = 0;
						break;
					}
				}
				if (!silent) printf("Communication problem. Closing connection.\n");
			}
		}
	}

	return EXIT_SUCCESS;
}
