# Makefile: MPT I/O library
LIB = mptio
#
# additional header directories
INC = . .. ../mptplot ../mptcore
#
# header to export
HEADER = notify.h stream.h
#
# source summary
sources = connect.c init.c output_new.c output_local.c newline_string.c
SRCS := $(wildcard */*.c) ${sources}
#
# shared library options
SHLIB_MAJOR = 1
SHLIB_MINOR = 0
SHLIB_TEENY = 0
SHLIB_OBJS = libinfo.o ${OBJS}
LDLIBS = -lmptcore
#
# import library creation settings
DIR_TOP = ${MPT_PREFIX}
DIR_INC = ${DIR_TOP}/include/mpt
include ../mpt.lib.mk
#
# additional dependencies
libinfo.o : ../libinfo.h ../version.h
CLEAR_FILES += libinfo.o
CLEAN_FILES += libinfo.o
