# Makefile: MPT I/O library example programs
#
DIR_BIN = .

progs = \
	swrite sread \
	mserver mclient \
	pipe

TESTS = $(tests:%=${DIR_BIN}/%)
PROGS = $(progs:%=${DIR_BIN}/%) ${TESTS}
LDLIBS = -lmptio

# general creation rules
all : ${PROGS}

# additional libraries needed
sread mclient : LDLIBS+=-lmptcore

# general targets and definitions
include ../mpt.example.mk

# additional targets
static : ${PROGS}
static : LDFLAGS+=-static
static : LDLIBS=-lmpt
