# Makefile: MPT base example programs
#

DIRS = core io cxx lua
TESTS = client axis iter
PROGS = readline ${TESTS}
OBJS = $(PROGS:%=%.o)

DIR_TOP = ${MPT_PREFIX}
include ../mpt.example.mk

# library config
client : LDLIBS=-lmpt++ -lmptio -lmptcore
readline : LDLIBS=-lmptloader
axis iter : LDLIBS=-lmptplot -lmptcore
%_static : LDLIBS=-lmpt
readline_static : LDLIBS=-lmpt -ldl

# special test arguments
test_axis : ARGS=title='my axis' intervals=3 tpos=Up
test_iter : ARGS='fact 2e-3 3 2 1' 'val= 1 2 3'

