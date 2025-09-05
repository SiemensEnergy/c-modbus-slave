#include "modbus.h"
#include "server.h"

#include <mbadu_tcp.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

enum {DEFAULT_MAX_NUM_CONNS=4};

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
	fprintf(stderr, " -p <port>       Use <port> as TCP port (default %d)\n", MBTCP_PORT);
	fprintf(stderr, " -n <num>        Maximum number of simultaneous connections (default %d)\n", DEFAULT_MAX_NUM_CONNS);
	fprintf(stderr, " -s              Do not print action logs\n");
}

int main(int argc, char *argv[])
{
	(void)argc;

	const char *cmd = *argv;

	int port = MBTCP_PORT;
	size_t max_ncs = DEFAULT_MAX_NUM_CONNS;
	int silent = 0;

	int ss, s;
	int is_new_conn;

	int *cs;
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
		} else if (!strcmp(*argv, "-n")) {
			if (!*++argv) {
				usage(cmd);
				fatal("Option -n must be followed by a number");
			}
			max_ncs = (size_t)atol(*argv);
		} else if (!strcmp(*argv, "-s")) {
			silent = 1;
		} else {
			usage(cmd);
			fatal("Unknown option %s", *argv);
		}
	}

	if (!(cs=calloc(max_ncs, sizeof cs[0]))) {
		fatal("Out of memory");
	}

	if (!silent) printf("Starting server on port %d with maximum %zu connection(s).\n", port, max_ncs);
	ss = server_init(port);
	if (ss<0) {
		fatal("Failed starting server on port %d", port);
	}

	modbus_init();

	while (1) {
		s = server_poll(ss, cs, max_ncs, &is_new_conn);

		if (is_new_conn) {
			for (ncs = 0; ncs<max_ncs; ++ncs) {
				if (!cs[ncs]) {
					cs[ncs] = s;
					if (!silent) printf("New connection.\n");
					break;
				}
			}
			if (ncs>=max_ncs) {
				server_close(s);
				if (!silent) printf("New connection rejected. Maximum number of connections (%zu) reached.\n", max_ncs);
			}
		} else if (s>0) {
			nrxbuf = server_recv(s, rxbuf, sizeof rxbuf);

			if (nrxbuf>0) {
				if ((ntxbuf=mbadu_tcp_handle_req(modbus_get(), rxbuf, (size_t)nrxbuf, txbuf))>0) {
					(void)server_send(s, txbuf, ntxbuf);
				} else {
					server_close(s);
					for (ncs = 0; ncs<max_ncs; ++ncs) {
						if (cs[ncs]==s) {
							cs[ncs] = 0;
							break;
						}
					}
					if (!silent) printf("Malformed packet received. Closing connection.\n");
				}
			} else {
				server_close(s);
				for (ncs = 0; ncs<max_ncs; ++ncs) {
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
