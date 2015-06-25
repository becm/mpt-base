# mpt.conf.mk: global settings
DIR_INC ?= ${MPT_PREFIX}/include/mpt
ARCH_LIBC ?= gnu
ARCH ?= $(shell uname -m)-$(shell uname -s | tr A-Z a-z)$(if $(strip ${ARCH_LIBC}),-${ARCH_LIBC})
DIR_LIB ?= ${MPT_PREFIX}/lib/${ARCH}
DIR_BIN ?= ${MPT_PREFIX}/bin
DIR_SHARE ?= ${MPT_PREFIX}/share

# get current directory
DIR_BASE ?= $(dir $(lastword $(MAKEFILE_LIST)))
#
# preprocessor flags
CPPFLAGS ?= -Wall -Werror -W $(INC:%=-I%) $(DEF:%=-D'%')
# compiler flags
CFLAGS ?= -fPIE -fPIC -g -pg -fstack-protector
CXXFLAGS ?= ${CFLAGS}
FFLAGS ?= -fpic -O5 -Wall
#
# install header files
define install_files
  $(if $(strip ${2}),@install -d '${1}' && install -C -m 644 ${2} '${1}' && printf 'install(%s): %s\n' '${1}' '${2}')
endef
