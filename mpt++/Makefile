# Makefile: MPT C++ library
LIB = mpt++
#
# additional header directories
INC = .. ../mptcore ../mptplot ../mptio
#
# header to export
HEADER = graphic.h io.h
#
# shared library options
SHLIB_MAJOR = 1
SHLIB_MINOR = 0
SHLIB_TEENY = 0
SHLIB_OBJS = libinfo.o ${OBJS}
LINK = ${CXX} ${CXXFLAGS}
LDLIBS = -lmptplot -lmptio -lmptcore -lm
#
# require C++11 for compilation
NEED_CXX_11 = cycle.o item.o graph.o layout.o linepart.o mapping.o meta_typed.o polyline.o
#
# source summary
SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:%.cpp=%.o)
#
# import library creation settings
DIR_TOP = ${MPT_PREFIX}
DIR_INC = ${DIR_TOP}/include/mpt
include ../mpt.lib.mk
#
${NEED_CXX_11} : CPPFLAGS+=-std=c++11
#
# additional dependencies
graphic.o mapping.o io_* : ${HEADER}
libinfo.o : ../libinfo.h ../version.h
#
# remove library info object
CLEAR_FILES += libinfo.o
CLEAN_FILES += libinfo.o
