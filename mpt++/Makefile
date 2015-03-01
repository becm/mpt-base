# Makefile: MPT C++ library
LIB = mpt++
#
# additional header directories
INC = .. ../mptcore ../mptplot
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
LDLIBS = -lmptplot -lmptcore
#
# source summary
SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:%.cpp=%.o)
#
# import library creation settings
include ../mpt.lib.mk
#
# additional dependencies
libinfo.o : ../libinfo.h ../version.h release.h
#
# import release creation settings
include ../mpt.release.mk
