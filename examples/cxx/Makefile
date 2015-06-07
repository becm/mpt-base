# Makefile: MPT C++ library example programs
#
DIR_BIN = .

tests = \
	object \
	buffer \
	config \
	queue \
	output \
	layout \
	map

progs = \
	comp

TESTS = $(tests:%=${DIR_BIN}/%)
PROGS = $(progs:%=${DIR_BIN}/%) ${TESTS}
LDLIBS = -lmpt++

# general creation rules
all : ${PROGS}

# additional libraries
output : LDLIBS+=-lmptio -lmptcore
comp   : LDLIBS+=-lmptio

# general targets and definitions
include ../mpt.example.mk

# additional targets
static : ${PROGS} ${DIR_BIN}/layout ${DIR_BIN}/comp
static : LDFLAGS+=-static
