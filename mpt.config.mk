# mpt.config.mk: global settings
ARCH ?=  $(shell ${CPP} -dumpmachine)
#
# target directory setup
PREFIX ?= $(dir $(lastword $(MAKEFILE_LIST)))build
DIR_INC ?= ${PREFIX}/include
DIR_LIB ?= ${PREFIX}/lib/${ARCH}
DIR_BIN ?= ${PREFIX}/bin
DIR_SHARE ?= ${PREFIX}/share
#
# warning flags for compiler
CPPWARN ?= all error extra format-security
# preprocessor flags
CPPFLAGS ?= $(CPPWARN:%=-W%) -pedantic $(INC:%=-I%) $(DEF:%=-D%)
# compiler flags
CFLAGS ?= -fPIE -fPIC -g -pg -fstack-protector
CXXFLAGS ?= ${CFLAGS}
# FFLAGS ?= -fpic -O5 -Wall
