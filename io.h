#ifndef IO_H
#define IO_H

#include <sys/types.h>
#include <stdio.h>

#define IO_TYPE_FD	1
#define IO_TYPE_FILE	2
#define IO_TYPE_BUF	3

typedef union _io io;
typedef struct _io_fd io_fd;
typedef struct _io_file io_file;
typedef struct _io_buf io_buf;
typedef ssize_t (*io_fdfunc)(int, void *, size_t);
typedef size_t (*io_filefunc)(void *, size_t, size_t, void *);

struct _io_fd {
	int type;
	int fd;
	io_fdfunc write;
	io_fdfunc read;
};

struct _io_file {
	int type;
	void *file;
	io_filefunc fwrite;
	io_filefunc fread;
};

struct _io_buf {
	int type;
	unsigned char *buf;
	size_t cap;
	size_t len;
	size_t off;
};

union _io {
	int type;
	io_fd fd;
	io_file file;
	io_buf buf;
};

void io_set_fd(io *io, int fd, io_fdfunc write, io_fdfunc read);
void io_set_file(io *io, void *file, io_filefunc fwrite, io_filefunc fread);
void io_set_buf(io *io);

size_t io_write(io *s, void *buf, size_t len);
size_t io_read(io *s, void *buf, size_t len);

int io_getc(io *s);
size_t io_putc(io *s, unsigned char c);
ssize_t io_getline(io *s, unsigned char *line, size_t len);
size_t io_printf(io *s, const char *fmt, ...);

#endif
