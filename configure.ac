AC_INIT([nntp], [0.1], [ivar.jarlsby@gmail.com])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_PROG_CC
AM_PROG_CC_C_O
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_PROG_LIBTOOL

AC_CONFIG_FILES([
	Makefile
	msg/Makefile
	tcp/Makefile
	io/Makefile
	parse/Makefile
	cmd/Makefile
	nntp.pc
])
AC_OUTPUT
