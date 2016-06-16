# Makefile: MPT C++ library
LIB = mpt++
#
# additional header directories
INC = .. ../mptcore ../mptplot ../mptio
#
# header to export
HEADER = object.h graphic.h
#
# shared library options
SHLIB_MAJOR = 1
SHLIB_MINOR = 0
SHLIB_TEENY = 0
SHLIB_OBJS = libinfo.o ${OBJS}
LINK = ${CXX} ${CXXFLAGS}
LDLIBS = -lmptplot -lmptio -lmptcore -lm
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
# use C++11 for compilation
CXX += -std=c++11
#
# additional dependencies
libinfo.o : ../libinfo.h ../version.h
CLEAN_FILES += libinfo.o
