# Makefile: MPT C++ library example programs
#
CXX11_TARGETS = buffer map object
TESTS = \
	${CXX11_TARGETS} \
	array \
	config \
	layout \
	output \
	queue \
	types

PROGS = comp ${TESTS}
LDLIBS = -lmpt++

${PROGS} $(PROGS:%=%_static) : CC=${CXX}

# general targets and definitions
DIR_TOP = ${MPT_PREFIX}
include ../../mpt.example.mk
CLEAN_FILES += ${CLEAR_FILES} $(PROGS:%=%.o)

# library config
config : LDLIBS+=-lmptcore
output : LDLIBS+=-lmptcore
comp : LDLIBS+=-lmptio
%_static : LDLIBS=-lmpt++ -lmpt
%_static : CPPFLAGS+=-D_STATIC
types : LDLIBS+=-lmptcore

# special test arguments
test_config : ARGS=mpt_* xdg_*
test_layout : ARGS=layout.lay
test_buffer : ARGS=1 2 3

# default to C++11 standard
$(CXX11_TARGETS:%=%.o) : CPPFLAGS+=-std=c++11
