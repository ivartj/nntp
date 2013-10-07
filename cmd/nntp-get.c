#include "../msg.h"
#include "../tcp.h"
#include "../io.h"
#include "../parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

char *server = "news.ux.uis.no";	/* presumably wont sue */
char *port = "119";
char *message_id = NULL;
char *outpath = NULL;
io *c;

void parseenv(void)
{
	char *envserver, *envport;

	envserver = getenv("NNTPSERVER");
	if(envserver != NULL)
		server = envserver;

	envport = getenv("NNTPPORT");
	if(envport != NULL)
		port = envport;
}

void usage(FILE *out)
{
	fprintf(out, "usage: nntp-get <message-id>\n");
}

void parseargs(int argc, char *argv[])
{
	int c;
	static struct option longopts[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "server", required_argument, NULL, 's' },
		{ "port", required_argument, NULL, 'p' },
		{ "output-file", required_argument, NULL, 'o' },
		{ 0, 0, 0, 0 },
	};

	while((c = getopt_long(argc, argv, "hs:p:o:", longopts, NULL)) != -1)
	switch(c) {
	case 'h':
		usage(stdout);
		exit(EXIT_SUCCESS);
	case 's':
		server = optarg;
		break;
	case 'p':
		port = optarg;
		break;
	case 'o':
		outpath = optarg;
		break;
	case ':':
	case '?':
		usage(stderr);
		exit(EXIT_FAILURE);
	}

	switch(argc - optind) {
	case 1:
		message_id = argv[optind];
		break;
	default:
		usage(stderr);
		exit(EXIT_FAILURE);
	}
}

void initconn(io *s)
{
	int c;

	c = tcp_dial(server, port);
	if(c == -1)
		msg_fail("tcp_dial: %s", msg_get());

	io_set_fd(s, c, tcp_write, tcp_read);
}

void greeting(io *s)
{
	size_t sz;
	int code;

	sz = parse_response_line(s, &code, NULL);
	if(sz == -1)
		msg_fail("parse_response_line: %s", msg_get());

	switch(code) {
	case 200:
	case 201:
		break;
	default:
		msg_fail("Received unexpected response code %d", code);
	}
}

void getarticle(io *s, io *out)
{
	int code;
	ssize_t sz;

	io_printf(s, "ARTICLE %s\r\n", message_id);

	sz = parse_response_line(s, &code, NULL);
	if(sz == -1)
		msg_fail("parse_response_line: %s", msg_get());

	switch(code) {
	case 220:
		break;
	default:
		msg_fail("Received unexpected response code %d", code);
	}

	sz = parse_response_block(s, out);
	if(sz == -1)
		msg_fail("parse_response_block: %s", msg_get());
}

void quit(void)
{
	io_printf(c, "QUIT\r\n");
}

void openfile(io *out)
{
	FILE *f;

	if(outpath == NULL) {
		io_set_fd(out, 1, (io_fdfunc)write, read);
		return;
	}

	f = fopen(outpath, "w");
	if(f == NULL)
		msg_fail("Failed to open file: %s", strerror(errno));

	io_set_file(out, (void *)f, (io_filefunc)fwrite, (io_filefunc)fread);
}

int main(int argc, char *argv[])
{
	io s, out;

	parseenv();
	parseargs(argc, argv);
	initconn(&s);
	greeting(&s);
	c = &s;
	atexit(quit);
	openfile(&out);
	getarticle(&s, &out);

	exit(EXIT_SUCCESS);
}
