# Makefile: MPT core functionality
LIB = mptplot
#
# additional header directories
INC = . .. ../mptcore
#
# header to export
HEADER = output.h plot.h values.h
#
# source summary
SRCS := $(wildcard */*.c)
#
# shared library options
SHLIB_MAJOR = 1
SHLIB_MINOR = 0
SHLIB_TEENY = 0
SHLIB_OBJS = libinfo.o
LDLIBS = -lmptcore
#
# import library creation settings
include ../mpt.lib.mk
# import release creation settings
RELEASE_HEADER = ../release.h
include ../mpt.release.mk
#
# additional dependencies
libinfo.o : ../libinfo.h ../version.h ${RELEASE_HEADER}
