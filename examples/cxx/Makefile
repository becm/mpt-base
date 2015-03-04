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

PROGS = $(progs:%=${DIR_BIN}/%)
TESTS = $(tests:%=${DIR_BIN}/%)
LDLIBS = -lmpt++

# general targets and definitions
include ../mpt.example.mk

output : LDLIBS+=-lmptio -lmptcore
comp   : LDLIBS+=-lmptio

# additional targets
static : ${PROGS} ${DIR_BIN}/layout ${DIR_BIN}/comp
static : LDFLAGS+=-static
