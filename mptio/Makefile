# Makefile: MPT I/O library
LIB = mptio
#
# additional header directories
INC = . .. ../mptplot ../mptclient ../mptcore
#
# header to export
HEADER = notify.h stream.h
#
# source summary
sources = connect.c init.c output_new.o readline.c
SRCS := $(wildcard */*.c) ${sources}
#
# shared library options
SHLIB_MAJOR = 1
SHLIB_MINOR = 0
SHLIB_TEENY = 0
SHLIB_OBJS = libinfo.o
LDLIBS = -lmptcore -ldl
#
# import library creation settings
include ../mpt.lib.mk
#
# additional dependencies
libinfo.o : ../libinfo.h ../version.h release.h
#
# import release creation settings
include ../mpt.release.mk
