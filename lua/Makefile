# Makefile: lua bindings for MPT
#
# lua version to build for
LUA = 5.3
#
# shared library version
LIB = lua${LUA}-mpt
#
# include lua and MPT core header
INC_LUA = /usr/include/lua${LUA}
INC = ${INC_LUA} ${DIR_BASE}mptcore ${DIR_BASE}mptio
#
# library sources
SRCS = mpt.c
OBJS = $(SRCS:%.c=%_lua${LUA}.o)
#
# shared library options
SHLIB_MAJOR = 1
SHLIB_MINOR = 0
SHLIB_TEENY = 0
LDLIBS = -lmptio
DIR_TOP = ${MPT_PREFIX}
include ../mpt.lib.mk
#
SHARE = mpt.lua
LUA_SHARE = ${DIR_SHARE}/lua/${LUA}
CLEAN_FILES += ${CLEAR_FILES}
CLEAR_FILES += $(SHARE:%=${LUA_SHARE}/%)
#
# service targets
.PHONY : install clear clean test devel shared
# file creation
install : $(SHARE:%=${LUA_SHARE}/%)

%_lua${LUA}.o : %.c
	${CC} ${CPPFLAGS} ${CFLAGS} -c -o ${@} ${<}

${LUA_SHARE}/% : %
	$(call install_files,${@D},${@F})
