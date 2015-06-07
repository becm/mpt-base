# Makefile: MPT core library example programs
#
DIR_BIN = .

progs = \
	parse

tests = \
	cobs \
	ident \
	axis \
	environ

TESTS = $(tests:%=${DIR_BIN}/%)
PROGS = $(progs:%=${DIR_BIN}/%) ${TESTS}
LDLIBS = -lmptcore

# general creation rules
all : ${PROGS}

# additional libraries needed
axis mclient : LDLIBS+=-lmptplot

# general targets and definitions
include ../mpt.example.mk

# additional targets
static : ${PROGS}
static : LDFLAGS+=-static
