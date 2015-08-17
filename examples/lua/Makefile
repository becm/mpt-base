# Makefile: MPT lua tests
#
# lua version to build for
LUA = 5.2
#
# include lua and MPT core header
INC_LUA ?= /usr/include/lua${LUA}
INC = ${INC_LUA} ${MPT_PREFIX}/include
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
# link flags and libraries
LDFLAGS = -L'${MPT_PREFIX_LIB}' -Wl,-R,'${MPT_PREFIX_LIB}'
LDLIBS = -llua${LUA} -llua${LUA}-mpt
#
# math environment setup
MBOX = ${MPT_PREFIX}/share/lua/${LUA}/mathbox.lua
#
# file creation
${PROG} : ${OBJS}
	${CC} ${LDFLAGS} -o ${@} ${OBJS} ${LDLIBS}

${PROG}_static : ${SRCS}
	${CC} -static -D_STATIC ${CPPFLAGS} ${LDFLAGS} -o ${@} ${SRCS} -llua${LUA} -ldl -lm

%_lua${LUA}.o : %.c
	${CC} ${CPPFLAGS} ${CFLAGS} -c -o ${@} ${<}
#
# service targets
.PHONY : clear clean test all
test : ${PROG}
	env MPT_MATHBOX='${MBOX}' LUA_PATH='?.lua;${MPT_PREFIX}/share/lua/${LUA}/?.lua' \
		lua${LUA} -l test -e 'print(testmpt()); testmath([[user.lua]])'
	./${PROG} 'user.lua'

static : ${PROG}_static

clear :
	${RM} ${PROG} ${PROG}_static

distclean clean : clear
	${RM} ${OBJS}
