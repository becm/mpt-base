# Makefile: MPT I/O library
LIB = mptio
#
# additional header directories
INC = . ../mptcore
#
# header to export
HEADER = connection.h notify.h stream.h
#
# source summary
sources = \
	connect.c \
	input_create.c input_traits.c \
	output_remote.c
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
${OBJS} : ${HEADER}
libinfo.o : ../libinfo.h ../version.h
#
# remove library info object
CLEAR_FILES += libinfo.o
CLEAN_FILES += libinfo.o
