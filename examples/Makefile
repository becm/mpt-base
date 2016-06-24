# Makefile: MPT base example programs
#

DIRS = core io cxx lua
TESTS = client axis iter gen
PROGS = readline ${TESTS}

client client_static : CC=${CXX}

DIR_TOP = ${MPT_PREFIX}
include ../mpt.example.mk
CLEAN_FILES += ${CLEAR_FILES} $(PROGS:%=%.o)

# library config
client : LDLIBS=-lmpt++ -lmptio -lmptcore
readline : LDLIBS=-lmptloader
axis iter gen : LDLIBS=-lmptplot -lmptcore
%_static : LDLIBS=-lmpt++ -lmpt
readline_static : LDLIBS=-lmpt -ldl

client : CPPFLAGS+=-std=c++11

# special test arguments
test_axis : ARGS=title='my axis' intervals=3 tpos=Up
test_iter : ARGS='fact 2e-3 3 2 1' 'val= 1 2 3'
test_gen  : ARGS='9 poly 1 0 0 : -4'

