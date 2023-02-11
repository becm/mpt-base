# Makefile: MPT loader library
LIB = mptloader
#
# additional header directories
INC = ../mptcore
#
# header to export
HEADER = loader.h
#
# source summary
SRCS := library_bind.c \
	library_open.c library_symbol.c \
	library_meta.c \
	readline.c
#
# shared library options
SHLIB_MAJOR = 1
SHLIB_MINOR = 0
SHLIB_TEENY = 0
SHLIB_OBJS = libinfo.o ${OBJS}
#
# import library creation settings
PREFIX_INC ?= ${PREFIX}/include/mpt
LDFLAGS = '-hlib${LIB}.so.${SHLIB_MAJOR}'
include ../mpt.lib.mk
#
# dynamic loader is extra library
ifeq ($(shell uname),Linux)
	LDLIBS = -ldl
endif
#
# additional dependencies
${OBJS} : ${HEADER}
libinfo.o : ../libinfo.h ../version.h
library_meta.o : ../mptcore/misc/refcount.c
#
# remove library info object
CLEAN_FILES += libinfo.o
CLEAN_FILES += libinfo.o
