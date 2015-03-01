# Makefile: MPT client operations
LIB = mptclient
#
# additional header directories
INC = . .. ../mptcore ../mptplot
#
# header to export
HEADER = client.h
#
# shared library options
SHLIB_MAJOR = 1
SHLIB_MINOR = 0
SHLIB_TEENY = 0
SHLIB_OBJS := libinfo.o
LDLIBS = -lmptcore -ldl
#
# source summary
subdirs = event loader
sources = init.c client_init.c readline.c
SRCS := ${sources} $(wildcard $(subdirs:%=%/*.c))
#
# import library creation settings
include ../mpt.lib.mk
#
# additional dependencies
libinfo.o : ../libinfo.h ../version.h release.h
#
# release information
include ../mpt.release.mk
