# Makefile: lua bindings for MPT
#
# lua version to build for
LUA = 5.2
#
LIB = ${DIR_LIB}/liblua${LUA}-mpt.so
#
# include lua and MPT core header
INC_LUA = /usr/include/lua${LUA}
INC = ${INC_LUA} ${DIR_BASE}mptcore ${DIR_BASE}mptplot ${DIR_BASE}mptio
#
# import global settings
DIR_TOP = ${MPT_PREFIX}
include ../config.mk
#
# library sources
SRCS = mpt.c
OBJS = $(SRCS:%.c=%_lua${LUA}.o)
#
# link parameters
LDFLAGS = -L${DIR_LIB} -zorigin -rpath=\$$ORIGIN
LDLIBS = -lmptio -lmptcore
#
SHARE = mpt.lua mathbox.lua
#
# service targets
.PHONY : install clear clean test devel shared
# file creation
shared devel : ${LIB}
install : ${LIB} $(SHARE:%=${DIR_SHARE}/mpt/%)
${LIB} : ${OBJS}
	${LD} -shared ${LDFLAGS} -o ${@} ${OBJS} ${LDLIBS}

%_lua${LUA}.o : %.c
	${CC} ${CPPFLAGS} ${CFLAGS} -c -o ${@} ${<}

${DIR_SHARE}/mpt/% :
	$(call install_files,${@D},${@F})

clear :
	${RM} ${LIB}

clean :
	${RM} ${OBJS}

distclean :
	${RM} ${LIB} ${OBJS} $(SHARE:%=${DIR_SHARE}/mpt/%)
