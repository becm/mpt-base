# Makefile: MPT I/O library
LIB = mptloader
#
# source summary
SRCS := library_assign.c library_bind.c library_open.c meta_open.c
OBJS := $(SRCS:%.c=%.o)
#
# import library creation settings
DIR_TOP = ${MPT_PREFIX}
DIR_INC = ${DIR_TOP}/include/mpt
include ../mpt.config.mk
#
# additional header directories
CPPFLAGS += -I../mptcore
#
${DIR_LIB}/libmptloader.a : ${OBJS}
	${AR} ${ARFLAGS} '${@}' $?

devel static : ${DIR_LIB}/libmptloader.a
shared :
