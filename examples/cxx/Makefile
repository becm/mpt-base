# Makefile: MPT C++ library example programs
#
CXX11_TARGETS = config map object layout
TESTS = \
	${CXX11_TARGETS} \
	array \
	buffer \
	iter \
	output \
	queue \
	types \
	values

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
iter : LDLIBS+=-lmptplot
comp : LDLIBS+=-lmptio
%_static : LDLIBS=-lmpt++ -lmpt
%_static : CPPFLAGS+=-D_STATIC

# special test arguments
test_config : ARGS=${MPT_PREFIX}
test_layout : ARGS=layout.lay
test_buffer : ARGS=1 2 3

# default to C++11 standard
${CXX11_TARGETS} $(CXX11_TARGETS:%=%_static) : CXXFLAGS+=-std=c++11
