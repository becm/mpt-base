# Makefile: MPT plotting library
LIB = mptcore
#
# additional header directories
INC = . .. ../mptplot ../mptio
#
# header to export
HEADER = $(wildcard *.h)
#
# source summary
SRCS := $(wildcard */*.c) abort.c identifier.c log.c mode_parse.c
#
# shared library options
SHLIB_MAJOR = 1
SHLIB_MINOR = 0
SHLIB_TEENY = 0
SHLIB_OBJS = libinfo.o ${OBJS}
#
# import library creation settings
DIR_TOP = ${MPT_PREFIX}
include ../lib.mk
# import release creation settings
include ../release.mk
#
# additional dependencies
libinfo.o : ../libinfo.h ../version.h ${RELEASE_HEADER}
CLEAN_FILES += libinfo.o
