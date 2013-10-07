SUBDIRS = . cmd

ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libnntp.la
# libnntp_la_LDFLAGS = -export-symbols-regex 'nntp_.*'
libnntp_la_SOURCES = msg.c msg.h tcp.c tcp.h io.c io.h parse.c parse.h nntp.c nntp.h

include_HEADERS = nntp.h

pkgconfigdir = $(includedir)/pkgconfig
pkgconfig_DATA = nntp.pc
