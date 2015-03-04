# Makefile: MPT I/O library
LIB = mptio
#
# additional header directories
INC = . .. ../mptplot ../mptcore
#
# header to export
HEADER = $(wildcard *.h)
#
# source summary
SRCS := $(wildcard */*.c) connect.c output_new.o
#
# shared library options
SHLIB_MAJOR = 1
SHLIB_MINOR = 0
SHLIB_TEENY = 0
SHLIB_OBJS = libinfo.o
LDLIBS = -lmptcore -lmptplot
#
# import library creation settings
include ../mpt.lib.mk
#
# additional dependencies
libinfo.o : ../libinfo.h ../version.h release.h
#
# import release creation settings
include ../mpt.release.mk
