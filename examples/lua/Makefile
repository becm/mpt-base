# Makefile: MPT lua tests
#
# lua version to build for
LUA = 5.3
#
# import MPT config
include ../../mpt.config.mk

# include lua and MPT core header
INC_LUA ?= /usr/include/lua${LUA}
INC = '${INC_LUA}'
#CPPFLAGS = $(INC:%=-I%)
#CFLAGS = -g
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

${PROG}_static : ${OBJS}
	${CC} -static ${LDFLAGS} -o ${@} ${OBJS} ${LDLIBS} -ldl -lm
#
# service targets
.PHONY : clear clean test all
test : ${PROG}
	LUA_PATH='?.lua;${PREFIX}/share/lua/${LUA}/?.lua' \
		lua${LUA} -l test -e 'print(testmpt()); testmath([[user.lua]])'
	MPT_PREFIX="${PREFIX}" ./${PROG} 'user.lua'

static : ${PROG}_static

clear :
	${RM} ${CLEAR_FILES}

distclean clean :
	${RM} ${CLEAN_FILES}
