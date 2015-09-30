# Makefile: MPT C++ library example programs
#
TESTS = \
	object \
	buffer \
	config \
	queue \
	output \
	layout \
	map

PROGS = comp ${TESTS}
LDLIBS = -lmpt++

# general targets and definitions
DIR_TOP = ${MPT_PREFIX}
include ../../mpt.example.mk

# library config
config : LDLIBS+=-lmptcore
output : LDLIBS+=-lmptio -lmptcore
comp : LDLIBS+=-lmptio
%_static : LDLIBS=-lmpt

# special test arguments
test_layout : ARGS=layout.lay

# default to C++11 standard
CPPFLAGS += -std=c++11
