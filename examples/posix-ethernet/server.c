#include"server.h"

#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/ip.h>

#include <stdint.h>
#include <stdlib.h>

extern int server_init(int port)
{
	int ss;
	struct sockaddr_in sin={0};

	if ((ss=socket(AF_INET, SOCK_STREAM, 0))==-1) {
		return -1;
	}

	sin.sin_family=AF_INET;
	sin.sin_addr.s_addr=INADDR_ANY;
	sin.sin_port=htons(port);

	if (bind(ss, (struct sockaddr*)&sin, sizeof sin)==-1 || listen(ss, 3)==-1) {
		close(ss);
		return -1;
	}

	return ss;
}

extern int server_poll(int ss, const int *cs, size_t ncss, int *is_new_conn)
{
	int s;
	struct timeval tv={0};
	size_t n;
	int maxs=0;

	fd_set read_fds;

	if (is_new_conn) *is_new_conn=0;

	FD_ZERO(&read_fds);
	FD_SET(ss, &read_fds);
	maxs = ss;

	for (n=0u; n<ncss; ++n) {
		if (cs[n]) FD_SET(cs[n], &read_fds);
		if (cs[n]>maxs) {
			maxs = cs[n];
		}
	}

	tv.tv_sec=0;
	tv.tv_usec=1000;

	if (select(maxs+1, &read_fds, NULL, NULL, &tv)==-1) {
		return -1;
	}

	if (FD_ISSET(ss, &read_fds)) {
		if ((s=accept(ss, NULL, NULL))!=-1) {
			if (is_new_conn) *is_new_conn=1;
			return s;
		}
	} else {
		for (n=0u; n<ncss; ++n) {
			if (FD_ISSET(cs[n], &read_fds)) {
				return cs[n];
			}
		}
	}

	return 0;
}

extern ssize_t server_recv(int s, uint8_t *buf, size_t len)
{
	return recv(s, buf, len, 0);
}

extern ssize_t server_send(int s, const uint8_t *buf, size_t len)
{
	return send(s, buf, len, 0);
}

extern int server_close(int s)
{
	/* TODO: Consider shutdown socket */
	return close(s);
}
