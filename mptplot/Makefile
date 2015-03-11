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
#
# additional dependencies
RELEASE_HEADER = ../release.h
libinfo.o : ../libinfo.h ../version.h ${RELEASE_HEADER}
#
# import release creation settings
include ../mpt.release.mk
