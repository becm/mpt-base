# Makefile: MPT core library example programs
#
TESTS = cobs ident environ
PROGS = parse ${TESTS}

LDLIBS = -lmptcore

# general targets and definitions
DIR_TOP = ${MPT_PREFIX}
include ../example.mk

# library config
%_static : LDLIBS=-lmpt

# special test arguments
test_ident : ARGS=1024 1025
test_environ : ARGS='mpt_*'
