# Makefile: MPT lua tests
#
# lua version to build for
LUA = 5.3
#
# import MPT config
DIR_TOP ?= ${MPT_PREFIX}
include ../../mpt.config.mk

# include lua and MPT core header
INC_LUA ?= /usr/include/lua${LUA}
INC = ${INC_LUA} ${DIR_INC}
CPPFLAGS = $(INC:%=-I'%')
CFLAGS = -g
#
# program sources
SRCS = main.c
OBJS = $(SRCS:%.c=%_lua${LUA}.o)
#
# program name
PROG = interp${LUA}
#
# generated files to remove
CLEAR_FILES = ${PROG} ${PROG}_static
CLEAN_FILES = ${CLEAR_FILES} ${OBJS}
#
# link flags and libraries
LDFLAGS = -L'${DIR_LIB}' -Wl,-R,'${DIR_LIB}'
LDLIBS = -llua${LUA}
#
# file creation
${PROG} : ${OBJS}
	${CC} ${LDFLAGS} -o ${@} ${OBJS} ${LDLIBS}

${PROG}_static : ${SRCS}
	${CC} -static -D_STATIC ${CPPFLAGS} ${CFLAGS} ${LDFLAGS} -o ${@} ${SRCS} ${LDLIBS} -lmpt -ldl -lm

%_lua${LUA}.o : %.c
	${CC} ${CPPFLAGS} ${CFLAGS} -c -o ${@} ${<}
#
# service targets
.PHONY : clear clean test all
test : ${PROG}
	env LUA_PATH='?.lua;${DIR_TOP}/share/lua/${LUA}/?.lua' \
		lua${LUA} -l test -e 'print(testmpt()); testmath([[user.lua]])'
	./${PROG} 'user.lua'

static : ${PROG}_static

clear :
	${RM} ${CLEAR_FILES}

distclean clean :
	${RM} ${CLEAN_FILES}
