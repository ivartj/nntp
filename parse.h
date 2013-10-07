#ifndef PARSE_H
#define PARSE_H

#include "io.h"

ssize_t parse_response_line(io *s, int *retcode, unsigned char line[512]);
ssize_t parse_response_block(io *s, io *buf);

#endif
