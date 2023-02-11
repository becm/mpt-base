# mpt.config.mk: global settings
ARCH ?=  $(shell ${CPP} -dumpmachine)
#
# set base source directory
MPT_BASE ?= $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
#
# target directory setup
PREFIX ?= $(abspath $(dir $(lastword $(MAKEFILE_LIST))))/build
PREFIX_INC ?= ${PREFIX}/include
PREFIX_LIB ?= ${PREFIX}/lib/${ARCH}
PREFIX_BIN ?= ${PREFIX}/bin
PREFIX_SHARE ?= ${PREFIX}/share
#
# warning flags for compiler
CPPWARN ?= all error extra format-security
# preprocessor flags
CPPFLAGS ?= $(CPPWARN:%=-W%) -pedantic $(INC:%=-I%) $(DEF:%=-D%)
# compiler flags
CFLAGS_MODE ?=  -g -pg
CFLAGS ?= -fPIE -fPIC ${CFLAGS_MODE} -fstack-protector
CXXFLAGS ?= ${CFLAGS}
#FFLAGS ?= -fpic -O5 -Wall
