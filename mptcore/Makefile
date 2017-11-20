# Makefile: MPT plotting library
LIB = mptcore
#
# additional header directories
INC = . ..
#
# header to export
HEADER =\
	array.h core.h client.h config.h convert.h \
	event.h message.h meta.h node.h \
	object.h output.h \
	parse.h queue.h
#
# source summary
SRCS := $(wildcard */*.c)
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
${OBJS} : ${HEADER}
object/object_set_value.o object/object_set_iterator.o : object/meta_wrap.h
libinfo.o : ../libinfo.h ../version.h
#
# remove library info object
CLEAR_FILES += libinfo.o
CLEAN_FILES += libinfo.o
