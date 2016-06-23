# Makefile: MPT plotting library
LIB = mptcore
#
# additional header directories
INC = . ..
#
# header to export
HEADER =\
	array.h core.h client.h config.h convert.h \
	event.h message.h node.h parse.h queue.h
#
# source summary
sources = abort.c identifier.c mode_parse.c reference.c
SRCS := $(wildcard */*.c) ${sources}
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
CLEAR_FILES += libinfo.o
CLEAN_FILES += libinfo.o
