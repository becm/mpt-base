# Makefile: MPT core functionality
LIB = mptplot
#
# additional header directories
INC = . .. ../mptcore
#
# header to export
HEADER = layout.h values.h
#
# source summary
SRCS := \
	history_push.c history_values.c \
	layout_open.c \
	mapping.c \
	output_local.c \
	$(wildcard */*.c)
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
