# Makefile: MPT C++ library example programs
#
DIR_BIN = .

programs = \
	object \
	buffer \
	config \
	queue \
	output \
	layout \
	map

interact = \
	comp

PROGS = $(programs:%=${DIR_BIN}/%)
INTERACT = $(interact:%=${DIR_BIN}/%)
LDLIBS = -lmpt++

CORE = static output comp ident

# general creation rules
all     : ${PROGS} ${INTERACT}
output  : LDLIBS+=-lmptplot
${CORE} : LDLIBS+=-lmptcore
graphic : LDLIBS=-lmptQt -lmpt++ $(shell pkg-config --libs QtGui)
graphic : CPPFLAGS+=$(shell pkg-config --cflags QtGui QtNetwork)
clear clean : PROGS+=${INTERACT}

# general targets and definitions
include ../mpt.example.mk

# additional targets
static : ${PROGS} ${DIR_BIN}/layout ${DIR_BIN}/comp
static : LDFLAGS+=-static
