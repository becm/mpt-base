# Makefile: lua bindings for MPT
#
# lua version to build for
LUA = 5.2
#
LIB = ${DIR_LIB}/liblua${LUA}-mpt.so
#
# include lua and MPT core header
INC = /usr/include/lua${LUA} ${DIR_BASE}mptcore ${DIR_BASE}mptplot ${DIR_BASE}mptio
#
# import global settings
include ../mpt.conf.mk
#
# library sources
SRCS = mpt.c
OBJS = $(SRCS:%.c=%_lua${LUA}.o)
#
# link parameters
LDFLAGS = -L${DIR_LIB} -R \$$ORIGIN
LDLIBS  = -lmptio -lmptcore
#
# service targets
.PHONY : clear clean test devel shared
# file creation
all devel : ${LIB} ${DIR_SHARE}/mpt/mpt.lua ${DIR_SHARE}/mpt/mathbox.lua
shared : ${LIB}
${LIB} : ${OBJS}
	${LD} -shared ${LDFLAGS} -o ${@} ${OBJS} ${LDLIBS}

%_lua${LUA}.o : %.c
	${CC} ${CPPFLAGS} ${CFLAGS} -c -o ${@} ${<}

${DIR_SHARE}/mpt/% :
	install -d '${@D}' && install -C -m 644 '${@F}' '${@D}'

clear :
	${RM} ${LIB}

clean : clear
	${RM} ${OBJS}
