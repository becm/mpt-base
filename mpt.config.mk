# mpt.config.mk: global settings
ARCH_OS != uname -s | tr A-Z a-z
ARCH_SYS != uname -m
ARCH_LIBC ?= gnu
ARCH ?= ${ARCH_SYS}-${ARCH_OS}$(if $(strip ${ARCH_LIBC}),-${ARCH_LIBC})
#
# target directory setup
DIR_TOP ?= /usr
DIR_INC ?= ${DIR_TOP}/include
DIR_LIB ?= ${DIR_TOP}/lib/${ARCH}
DIR_BIN ?= ${DIR_TOP}/bin
DIR_SHARE ?= ${DIR_TOP}/share
#
# warning flags for compiler
CPPWARN ?= all error extra format-security
# preprocessor flags
CPPFLAGS ?= $(CPPWARN:%=-W%) -pedantic $(INC:%=-I%) $(DEF:%=-D%)
# compiler flags
CFLAGS ?= -fPIE -fPIC -g -pg -fstack-protector
CXXFLAGS ?= ${CFLAGS}
# FFLAGS ?= -fpic -O5 -Wall
