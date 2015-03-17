# Makefile: MPT lua tests
#
# lua version to build for
LUA = 5.2
#
# include lua and MPT core header
INC = /usr/include/lua${LUA} ${MPT_PREFIX}/include
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
MBOX = ${MPT_PREFIX}/share/mpt/mathbox.lua
#
# file creation
${PROG} : ${OBJS}
	${CC} ${LDFLAGS} -o ${@} ${OBJS} ${LDLIBS}

%_lua${LUA}.o : %.c
	${CC} ${CPPFLAGS} ${CFLAGS} -c -o ${@} ${<}
#
# service targets
.PHONY : clear clean test all
all : ${PROG}
test : ${PROG}
	env MPT_MATHBOX='${MBOX}' LUA_PATH='?.lua;${MPT_PREFIX}/share/mpt/?.lua' \
		lua${LUA} -l test -e 'print(testmpt()); testmath([[user.lua]])'
	env MPT_MATHBOX='${MBOX}' ./${PROG} 'user.lua'

clear :
	${RM} ${PROG}

clean : clear
	${RM} ${OBJS}
