# Makefile: MPT core library example programs
#
TESTS = coding config ident environ value
PROGS = parse ${TESTS}

LDLIBS = -lmptcore
INC = '${DIR_BASE}mptcore'

# general targets and definitions
DIR_TOP = ${MPT_PREFIX}
include ../../mpt.example.mk
CLEAN_FILES += ${CLEAR_FILES} $(PROGS:%=%.o)

# library config
%_static : LDLIBS=-lmpt
#
# special test arguments
test_ident : ARGS=28 29 252 253
test_environ : ARGS='mpt_*'
test_coding : ARGS=cobs
test_config : ARGS=prefix.lib

parse_tests = online layout_1 layout_2 subsect config
.PHONY : $(parse_tests:%=test_parse_%)
test : $(parse_tests:%=test_parse_%)
# see MPT_NAMEFLAG(Flags) in parse.h for name restriction configuration
test_parse_online : parse
	./parse online.txt  '{*} =;!# `' 'Ef'
test_parse_layout_1 : parse
	./parse layout.txt  '[*] = ' 'Esnw'
test_parse_layout_2 : parse
	./parse layout.txt  '[*] = !'
test_parse_subsect : parse
	./parse subsect.lay '{*} =;!#' 'E'
test_parse_config : parse
	./parse config.txt  '[ ] = #' 'Esc'
