# Makefile: MPT base example programs
#

DIRS = core io cxx lua
TESTS = client graph graphic axis iter gen libassign
PROGS = readline graphic ${TESTS}
STATIC = libassign_static

client_static : CC=${CXX}

DIR_TOP = ${MPT_PREFIX}
include ../mpt.example.mk
CLEAN_FILES += ${CLEAR_FILES} $(PROGS:%=%.o)

libassign : libassign.o
libassign : LDLIBS=-lmptloader
libassign_static : LDLIBS=-lmpt -ldl

# library config
graph : LDLIBS=-lmpt++ -lmptplot -lmptcore
client : LDLIBS=-lmpt++ -lmptio -lmptcore
readline : LDLIBS=-lmptloader
axis iter gen : LDLIBS=-lmptplot -lmptcore
%_static : LDLIBS=-lmpt++ -lmpt
readline_static : LDLIBS=-lmpt -ldl
graphic : LDLIBS=-lmpt++
#
# object parameters
client.o : CPPFLAGS+=-std=c++11
#
# use C++ linker
client client_static : CC=${CXX}
graphic graphic_static : CC=${CXX}
#
# special test arguments
test_axis :      ARGS=title='my axis' intervals=3 tpos=Up
test_graph :     ARGS='w1 w2' 'ax'
test_client :    ARGS=-vv
test_gen :       ARGS=9 'poly 1 0 0 : -4' 'bound 1 0 2' 'lin 1 0'
test_iter :      ARGS='fac(8:1:.5:2)' '1 6 8' 'lin(20:0 180)'
test_libassign : ARGS='mpt_output_remote@libmptio.so.1'

