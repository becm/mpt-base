# Makefile: MPT loader library
LIB = mptloader
#
# additional header directories
INC = .. ../mptcore
#
# source summary
SRCS := library_assign.c library_bind.c library_open.c \
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
DIR_TOP = ${MPT_PREFIX}
include ../mpt.lib.mk
#
# dynamic loader is extra library
ifeq (${ARCH_OS},linux)
	LDLIBS = -lmptcore -ldl
else
	LDLIBS = -lmptcore
endif
#
# additional dependencies
libinfo.o : ../libinfo.h ../version.h
CLEAN_FILES += libinfo.o
CLEAN_FILES += libinfo.o
