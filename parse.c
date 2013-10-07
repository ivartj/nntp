#include "parse.h"
#include "msg.h"
#include <string.h>

ssize_t parse_response_line(io *s, int *retcode, unsigned char linecpy[512])
{
	/* 512 is maximum length of response line */
	int i, j;
	unsigned char line[512] = { 0 };
	int c;

	for(i = 0; i < 512; i++) {
		c = io_getc(s);
		switch(c) {
		case EOF:
			msg_set("Unexpected end of stream in response line");
			return -1;
		case '\n':
			line[i] = '\0'; /* intentional */
			goto ret;
		default:
			line[i] = (unsigned char)c;
			break;
		}
	}
	if(i == 512) {
		msg_set("Response line exceeding maximum 512 bytes");
		return -1;
	}
	if(i < 4) {
		msg_set("Response line too small to contain response code");
		return -1;
	}
ret:
	*retcode = 0;

	for(j = 0; j < 3; j++) {
		c = line[j];
		if(c < '0' || c > '9') {
			msg_set("Response line did not contain 3-digit response code");
			return -1;
		}
		*retcode *= 10;
		*retcode += c - '0';
	}

	if(linecpy != NULL)
		memcpy(linecpy, line, sizeof(line));

	return i;
}

ssize_t parse_response_block(io *s, io *buf)
{
	int c;
	enum {
		meat,
		line,
		dot,
		dot_r,
	} state;
	size_t sz = 0;

	state = line;
	while((c = io_getc(s)) != EOF) {
		sz += 1;
		switch(state) {
		case meat:
			switch(c) {
			case '\n':
				io_putc(buf, '\n');
				state = line;
				break;
			default:
				io_putc(buf, c);
				break;
			}
			break;
		case line:
			switch(c) {
			case '.':
				state = dot;
				break;
			case '\n':
				io_putc(buf, '\n');
				break;
			default:
				state = meat;
				io_putc(buf, c);
				break;
			}
			break;
		case dot:
			switch(c) {
			case '\r':
				state = dot_r;
				break;
			case '\n':
				goto ret;
			case '.':
				state = meat;
				io_putc(buf, '.');
				break;
			default:
				state = meat;
				io_putc(buf, '.');
				io_putc(buf, c);
				break;
			}
			break;
		case dot_r:
			switch(c) {
			case '\n':
				goto ret;
			default:
				state = meat;
				io_putc(buf, '.');
				io_putc(buf, '\r');
				io_putc(buf, c);
				break;
			}
		}
	}
	if(c == EOF) {
		msg_set("Unexpected end of stream in response block");
		return -1;
	}
ret:
	return sz;
}
