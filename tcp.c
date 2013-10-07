#include "tcp.h"
#include "msg.h"
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int tcp_dial(const char *node, const char *port)
{
	int c;
	int err;
	struct addrinfo hints = { 0 }, *res = NULL, *iter;

	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	err = getaddrinfo(node, port, &hints, &res);
	if(err) {
		msg_set("getaddrinfo: %s", gai_strerror(err));
		return -1;
	}

	if(res == NULL) {
		msg_set("getaddrinfo: No matches returned");
		return -1;
	}

	for(iter = res; iter != NULL; iter = iter->ai_next) {
		c = socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol);
		if(c == -1) {
			msg_set("socket: %s", strerror(errno));
			continue;
		}
		err = connect(c, iter->ai_addr, iter->ai_addrlen);
		if(err) {
			msg_set("connect: %s", strerror(errno));
			close(c);
			continue;
		}
		break;
	}

	if(iter == NULL)
		c = -1;

	freeaddrinfo(res);
	return c;
}

ssize_t tcp_write(int c, void *buf, size_t bytes)
{
	return send(c, buf, bytes, 0);
}

ssize_t tcp_read(int c, void *buf, size_t bytes)
{
	return recv(c, buf, bytes, 0);
}

void tcp_close(int c)
{
	close(c);
}
