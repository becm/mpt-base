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
