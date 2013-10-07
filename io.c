#include "io.h"
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

void io_set_fd(io *s, int fd, io_fdfunc write, io_fdfunc read)
{
	memset(s, 0, sizeof(io));
	s->fd.type = IO_TYPE_FD;
	s->fd.fd = fd;
	s->fd.write = write;
	s->fd.read = read;
}

void io_set_file(io *s, void *file, io_filefunc fwrite, io_filefunc fread)
{
	memset(s, 0, sizeof(io));
	s->file.type = IO_TYPE_FILE;
	s->file.file = file;
	s->file.fwrite = fwrite;
	s->file.fread = fread;
}

void io_set_buf(io *s)
{
	memset(s, 0, sizeof(io));
	s->buf.type = IO_TYPE_BUF;
}

size_t io_write(io *s, void *data, size_t len)
{
	ssize_t n;
	int szchange;


	switch(s->type) {
	case IO_TYPE_FD:
		n = s->fd.write(s->fd.fd, data, len);
		if(n == -1)
			return 0;
		return n;
	case IO_TYPE_FILE:
		return s->file.fwrite(data, 1, len, s->file.file);
	case IO_TYPE_BUF:
		szchange = 0;
		while(s->buf.len + len > s->buf.cap) {
			if(s->buf.cap == 0)
				s->buf.cap = 256;
			else
				s->buf.cap *= 2;
			szchange = 1;
		}
		if(szchange)
			s->buf.buf = realloc(s->buf.buf, s->buf.cap);
		memcpy(s->buf.buf + s->buf.len, data, len);
		s->buf.len += len;
		return len;
	}

	return 0;
}

size_t io_read(io *s, void *buf, size_t len)
{
	ssize_t n;

	switch(s->type) {
	case IO_TYPE_FD:
		n = s->fd.read(s->fd.fd, buf, len);
		if(n == -1)
			return 0;
		return n;
	case IO_TYPE_FILE:
		return s->file.fread(buf, 1, len, s->file.file);
	case IO_TYPE_BUF:
		if(len > s->buf.len - s->buf.off)
			len = s->buf.len - s->buf.off;
		memcpy(buf, s->buf.buf + s->buf.off, len);
		s->buf.off += len;
		return len;
	}

	return 0;
}

int io_getc(io *s)
{
	unsigned char c;
	size_t len;

	len = io_read(s, &c, sizeof(c));
	if(len == 0)
		return EOF;
	return c;
}

size_t io_putc(io *s, unsigned char c)
{
	return io_write(s, &c, sizeof(c));
}

ssize_t io_getline(io *s, unsigned char *line, size_t len)
{
	size_t i;
	int c;

	for(i = 0; i < len - 1; i++) {
		c = io_getc(s);
		switch(c) {
		case EOF:
			if(i == 0)
				return -1;
			/* fallthrough */
		case '\n':
			line[i] = '\0';
			return i;
		default:
			line[i] = (unsigned char)c;
		}
	}
	line[i] = '\0';

	return i;
}

size_t io_printf(io *s, const char *fmt, ...)
{
	va_list ap;
	char *buf = NULL;
	size_t len;

	va_start(ap, fmt);
	len = vasprintf(&buf, fmt, ap);
	io_write(s, buf, len);
	free(buf);
	va_end(ap);

	return len;
}
