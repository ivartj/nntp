#ifndef TCP_H
#define TCP_H

#include <sys/types.h>

int tcp_dial(const char *node, const char *port);
ssize_t tcp_write(int c, void *buf, size_t bytes);
ssize_t tcp_read(int c, void *buf, size_t bytes);
void tcp_close(int c);

#endif
