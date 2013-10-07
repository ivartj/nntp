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
char *group = NULL;
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
	fprintf(out, "usage: nntp-latest <newsgroup>\n");
}

void parseargs(int argc, char *argv[])
{
	int c, m;
	static struct option longopts[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "server", required_argument, NULL, 's' },
		{ "port", required_argument, NULL, 'p' },
		{ "maximum-articles", required_argument, NULL, 'n' },
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
		group = argv[optind];
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

int processline(io *s)
{
	int code;
	int m;
	ssize_t sz;
	char line[512], msgid[512];

	sz = parse_response_line(s, &code, line);
	if(sz == -1)
		msg_fail("parse_response_line: %s", msg_get());
	switch(code) {
	case 223:
		break;
	case 422:
		exit(EXIT_SUCCESS);
	default:
		msg_fail("Received unexpected response code %d", code);
	}

	m = sscanf(line, "%*d %*d %s", msgid);
	if(m == 0)
		msg_fail("Failed to parse message-id from response");
	printf("%s\n", msgid);

	return 0;
}

void getlatest(io *s)
{
	ssize_t sz;
	int code;
	int high;
	int i;
	char line[512], msgid[512];
	int m;
	int n;

	io_printf(s, "GROUP %s\r\n", group);

	sz = parse_response_line(s, &code, line);
	if(sz == -1)
		msg_fail("parse_response_line: %s", msg_get());

	switch(code) {
	case 211:
		break;
	case 411:
		msg_fail("No such newsgroup");
	default:
		msg_fail("Received unexpected response code %d", code);
	}

	m = sscanf(line, "%*d %*d %*d %d", &high);
	if(m == 0)
		msg_fail("Failed to parse latest article number from response line");

	io_printf(s, "STAT %d\r\n", high);
	processline(s);
	for(i = 1; i < max; i++)
		io_printf(s, "LAST\r\n");
	for(i = 1; i < max; i++)
		processline(s);
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
	getlatest(&s);

	exit(EXIT_SUCCESS);
}
