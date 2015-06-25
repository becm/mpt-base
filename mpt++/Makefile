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
SHLIB_OBJS = libinfo.o
LINK = ${CXX}
LDLIBS = -lmptplot -lmptio -lmptcore -lm
#
# source summary
SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:%.cpp=%.o)
#
# import library creation settings
include ../mpt.lib.mk
# import release creation settings
RELEASE_HEADER = ../release.h
include ../mpt.release.mk
#
# additional dependencies
libinfo.o : ../libinfo.h ../version.h ${RELEASE_HEADER}
