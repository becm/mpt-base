# Makefile: MPT plotting library
LIB = mptcore
#
# additional header directories
INC = . ..
#
# header to export
HEADER =\
	array.h \
	client.h collection.h config.h convert.h core.h \
	event.h \
	message.h meta.h \
	node.h \
	object.h output.h \
	parse.h \
	types.h \
	queue.h
#
# source summary
SRCS := $(wildcard */*.c)
#
# shared library options
SHLIB_MAJOR = 1
SHLIB_MINOR = 0
SHLIB_TEENY = 0
SHLIB_OBJS = libinfo.o ${OBJS}
# avoid runtime path extension
LDFLAGS = '-hlib${LIB}.so.${SHLIB_MAJOR}'
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
