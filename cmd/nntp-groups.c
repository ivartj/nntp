#include "../msg.h"
#include "../tcp.h"
#include "../io.h"
#include "../parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

int max = 50;				/* arbitrary */
char *server = "news.ux.uis.no";	/* presumably wont sue */
char *port = "119";
char *wildmat = NULL;
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
	fprintf(out, "usage: nntp-groups [ -n ] <wildmat>\n");
}

void parseargs(int argc, char *argv[])
{
	int c, m;
	static struct option longopts[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "server", required_argument, NULL, 's' },
		{ "port", required_argument, NULL, 'p' },
		{ "maximum-groups", required_argument, NULL, 'n' },
		{ 0, 0, 0, 0 },
	};

	while((c = getopt_long(argc, argv, "hs:p:n:", longopts, NULL)) != -1)
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
	case 'n':
		m = sscanf(optarg, "%u", &max);
		if(m == 0) {
			fprintf(stderr, "Invalid maximum article number '%s'", optarg);
			usage(stderr);
			exit(EXIT_FAILURE);
		}
		break;
	case ':':
	case '?':
		usage(stderr);
		exit(EXIT_FAILURE);
	}

	switch(argc - optind) {
	case 1:
		wildmat = argv[optind];
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

void groups(io *s)
{
	ssize_t sz;
	int code;
	io buf;
	unsigned char line[512];
	unsigned char group[512];

	io_printf(s, "LIST ACTIVE %s\r\n", wildmat);

	sz = parse_response_line(s, &code, NULL);
	if(sz == -1)
		msg_fail("parse_response_line: %s", msg_get());

	switch(code) {
	case 215:
		break;
	default:
		msg_fail("Received unexpected response code %d", code);
	}

	io_set_buf(&buf);

	sz = parse_response_block(s, &buf);
	if(sz == -1)
		msg_fail("parse_response_block: %s", msg_get());

	while((sz = io_getline(&buf, line, sizeof(line))) != -1) {
		sscanf(line, "%s", group);
		printf("%s\n", group);
	}
}

void quit(void)
{
	io_printf(c, "QUIT\r\n");
}

int main(int argc, char *argv[])
{
	io s;

	parseenv();
	parseargs(argc, argv);
	initconn(&s);
	c = &s;
	atexit(quit);
	greeting(&s);
	groups(&s);

	exit(EXIT_SUCCESS);
}
