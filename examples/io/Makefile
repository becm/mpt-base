# Makefile: MPT I/O library example programs
#
TESTS = pipe
PROGS = \
	swrite sread \
	mserver mclient \
	dispatch \
	${TESTS}

LDLIBS = -lmptio
INC = '${MPT_BASE}/mptio' '${MPT_BASE}/mptcore'

# general targets and definitions
include ../../mpt.example.mk
CLEAN_FILES += ${CLEAR_FILES} $(PROGS:%=%.o)

# library config
sread mclient dispatch : LDLIBS+=-lmptcore
%_static : LDLIBS=-lmpt

# special test arguments
test_pipe : ARGS=/bin/ls
