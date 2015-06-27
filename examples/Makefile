# Makefile: MPT base example programs
#

DIRS = core io cxx lua
TESTS = client axis
PROGS = readline ${TESTS}
OBJS = $(PROGS:%=%.o)

DIR_TOP = ${MPT_PREFIX}
include example.mk

# library config
client : LDLIBS=-lmpt++ -lmptio -lmptcore
readline : LDLIBS=-lmptio
axis : LDLIBS=-lmptplot -lmptcore
%_static : LDLIBS=-lmpt
readline_static : LDLIBS+=-ldl

# special test arguments
test_axis : ARGS=title='my axis' intervals=3 tpos=Up

