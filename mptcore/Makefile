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
SHLIB_OBJS = libinfo.o
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
