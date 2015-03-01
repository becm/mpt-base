# mpt.conf.mk: global settings
DIR_INC ?= ${MPT_PREFIX}/include/mpt
ARCH ?= $(shell uname -m)-$(shell uname -s | tr A-Z a-z)-gnu
DIR_LIB ?= ${MPT_PREFIX}/lib/${ARCH}
DIR_BIN ?= ${MPT_PREFIX}/bin

# get current directory
DIR_BASE := $(dir $(lastword $(MAKEFILE_LIST)))
#
# preprocessor flags
CPPFLAGS += -Wall -Werror -W $(INC:%=-I%) $(DEF:%=-D'%')
# compiler flags
CFLAGS += -fPIE -fPIC -g -pg -fstack-protector
CXXFLAGS ?= ${CFLAGS}
FFLAGS += -fpic -O5 -Wall
#
# install files
INST.d ?= install -d
INST.h ?= install -C -m 644 -t '${DIR_INC}'
#
