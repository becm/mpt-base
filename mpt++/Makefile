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
# special C++ features for compilation
NEED_CXX_AUTO = graph.o item.o layout.o linepart.o mapping.o
NEED_CXX_VA_COPY = logger.o
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
CXXFLAGS += -std=c++98
${NEED_CXX_AUTO} ${NEED_CXX_VA_COPY} : CXXFLAGS+=-std=c++11
#
# additional dependencies
graphic.o mapping.o io_* : ${HEADER}
libinfo.o : ../libinfo.h ../version.h
#
# remove library info object
CLEAR_FILES += libinfo.o
CLEAN_FILES += libinfo.o
