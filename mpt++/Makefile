# Makefile: MPT C++ library
LIB = mpt++
#
# additional header directories
INC = ../mptcore ../mptplot ../mptio
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
# special C++ features for compilation
NEED_CXX_AUTO = graph.o item_group.o layout.o linepart.o mapping.o
NEED_CXX_DECLTYPE = value.o
NEED_CXX_FOREACH = item_group.o
NEED_CXX_VA_COPY = logger.o
#
# source summary
SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:%.cpp=%.o)
#
# import library creation settings
PREFIX_INC ?= ${PREFIX}/include/mpt/
include ../mpt.lib.mk
#
CXXFLAGS += -std=c++98
${NEED_CXX_AUTO} ${NEED_CXX_VA_COPY} ${NEED_CXX_DECLTYPE} : CXXFLAGS+=-std=c++11
#
# additional dependencies
graphic.o mapping.o : graphic.h
io.o io_* : io.h
libinfo.o : ../libinfo.h ../version.h
#
# remove library info object
CLEAR_FILES += libinfo.o
CLEAN_FILES += libinfo.o
