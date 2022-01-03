# Makefile: MPT core library example programs
#
TESTS = coding config ident environ
PROGS = parse ${TESTS}

LDLIBS = -lmptcore
INC = '${DIR_BASE}mptcore'

io_inc = parse

# general targets and definitions
DIR_TOP = ${MPT_PREFIX}
include ../../mpt.example.mk
CLEAN_FILES += ${CLEAR_FILES} $(PROGS:%=%.o)

# library config
%_static : LDLIBS=-lmpt
#
# additional includes
$(io_inc:%=%) $(io_inc:%=%_static) : INC+='${DIR_BASE}mptio'
#
# special test arguments
test_ident : ARGS=28 29 252 253
test_environ : ARGS='mpt_*'
test_coding : ARGS=cobs
test_config : ARGS=prefix.lib
