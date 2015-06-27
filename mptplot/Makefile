# Makefile: MPT core functionality
LIB = mptplot
#
# additional header directories
INC = . .. ../mptcore
#
# header to export
HEADER = output.h plot.h values.h
#
# source summary
SRCS := $(wildcard */*.c)
#
# library options
SHLIB_MAJOR = 1
SHLIB_MINOR = 0
SHLIB_TEENY = 0
SHLIB_OBJS = libinfo.o ${OBJS}
LDLIBS = -lmptcore
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
