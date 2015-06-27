# Makefile: MPT I/O library example programs
#
TESTS = pipe
PROGS = \
	swrite sread \
	mserver mclient \
	${TESTS}

LDLIBS = -lmptio

# general targets and definitions
DIR_TOP = ${MPT_PREFIX}
include ../example.mk

# library config
sread mclient : LDLIBS+=-lmptcore
%_static : LDLIBS=-lmpt

# special test arguments
test_pipe : ARGS=/bin/ls
