# Makefile: lua bindings for MPT
#
# lua version to build for
LUA = 5.2
#
# include lua and MPT core header
INC = /usr/include/lua${LUA} ${DIR_BASE}mptcore ${DIR_BASE}mptplot
#
# import global settings
include ${MPT_PREFIX}/src/base/mpt.conf.mk
#
# library sources
SRCS = luampt.c
OBJS = $(SRCS:lua%.c=%_lua${LUA}.o)
IOBJS = main_lua${LUA}.o
#
# library destination
LIBRARY = ${DIR_LIB}/liblua${LUA}-mpt.so
INTERPRETER = ./interp${LUA}
#
# link parameters
LDFLAGS = -L${DIR_LIB} -R \$$ORIGIN
LDLIBS  = mptcore
#
# service targets
.PHONY : clear clean test devel shared
devel shared : ${LIBRARY}
test : ${LIBRARY} ${INTERPRETER}
	@lua${LUA} -l test -e 'print(testmpt()); testmath([[user.lua]])' && \
	${INTERPRETER} "user.lua"

clear :
	${RM} ${DIR_LIB}/liblua*-mpt.so interp*

clean : clear
	${RM} mpt_lua*.o main_*.o
#
# file creation
${LIBRARY} : ${OBJS}
	${LD} -shared ${LDFLAGS} -o ${@} ${OBJS} $(LDLIBS:%=-l%)

${INTERPRETER} : ${IOBJS} ${OBJS}
	${CC} -L${DIR_LIB} -o ${@} ${IOBJS} ${OBJS} -Wl,-R,${DIR_LIB} -llua${LUA} -lmptcore

%_lua${LUA}.o : lua%.c
	${CC} ${CPPFLAGS} ${CFLAGS} -c -o ${@} ${<}

main_lua${LUA}.o : main.c
	${CC} ${CPPFLAGS} ${CFLAGS} -c -o ${@} ${<}
