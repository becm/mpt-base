# Makefile: MPT C++ library example programs
#
CXX11_TARGETS = object buffer map
TESTS = \
	${CXX11_TARGETS} \
	config \
	queue \
	output \
	layout

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

# special test arguments
test_layout : ARGS=layout.lay

# default to C++11 standard
$(CXX11_TARGETS:%=%) : CPPFLAGS+=-std=c++11
$(CXX11_TARGETS:%=%.o) : CPPFLAGS+=-std=c++11
