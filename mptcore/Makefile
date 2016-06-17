# Makefile: MPT plotting library
LIB = mptcore
#
# additional header directories
INC = . ..
#
# header to export
HEADER = core.h \
	array.h convert.h config.h message.h node.h queue.h \
	client.h event.h parse.h
#
# source summary
SRCS := $(wildcard */*.c) abort.c reference.c identifier.c mode_parse.c
#
# shared library options
SHLIB_MAJOR = 1
SHLIB_MINOR = 0
SHLIB_TEENY = 0
SHLIB_OBJS = libinfo.o ${OBJS}
#
# import library creation settings
DIR_TOP = ${MPT_PREFIX}
DIR_INC = ${DIR_TOP}/include/mpt
include ../mpt.lib.mk
#
# additional dependencies
libinfo.o : ../libinfo.h ../version.h
CLEAN_FILES += libinfo.o
