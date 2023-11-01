# Makefile: MPT C++ library example programs
#
CXX11_TARGETS = config map object layout
TESTS = \
	${CXX11_TARGETS} \
	array \
	buffer \
	iter \
	meta \
	output \
	queue \
	types \
	value \
	value_store

PROGS = comp ${TESTS}
LDLIBS = -lmpt++
INC = '${MPT_BASE}/mptcore'

cxx_inc = buffer output meta queue
io_inc = buffer comp output queue
plot_inc = iter layout map object value_store


${PROGS} $(PROGS:%=%_static) : CC=${CXX}

# general targets and definitions
include ../../mpt.example.mk
CLEAN_FILES += ${CLEAR_FILES} $(PROGS:%=%.o)

# library config
config : LDLIBS+=-lmptcore
output : LDLIBS+=-lmptcore
iter : LDLIBS+=-lmptplot
comp : LDLIBS+=-lmptio
%_static : LDLIBS=-lmpt++ -lmpt
%_static : CPPFLAGS+=-D_STATIC
#
# additional includes
$(cxx_inc:%=%) $(cxx_inc:%=%_static) : INC+='${MPT_BASE}/mpt++'
$(io_inc:%=%) $(io_inc:%=%_static) : INC+='${MPT_BASE}/mptio'
$(plot_inc:%=%) $(plot_inc:%=%_static) : INC+='${MPT_BASE}/mptplot'
#
# special test arguments
test_config : ARGS="${PREFIX}/etc"
test_layout : ARGS=layout.lay
test_buffer : ARGS=1 2 3

# default to C++11 standard
${CXX11_TARGETS} $(CXX11_TARGETS:%=%_static) : CXXFLAGS+=-std=c++11
