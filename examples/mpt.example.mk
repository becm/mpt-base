# mpt.example.mk: template for MPT examples
#
# flags for compiler/preprocessor
CPPFLAGS += -I${MPT_PREFIX}/include -Wall -Werror
CFLAGS   ?= -g -fstack-protector
CXXFLAGS ?= ${CFLAGS}
FFLAGS   ?= ${CFLAGS} ${CPPFLAGS}
#
# flags for linker
LDDIRS  ?= ${MPT_PREFIX_LIB}
DLDIRS  ?= ${LDDIRS}
LDFLAGS += $(LDDIRS:%=-L%) $(DLDIRS:%=-Wl,-R%)
LINK    ?= ${CC}
#
# auto-generated content
OUTPUT ?= $(PROGS:%=%.out)
#
# general rules
.PHONY: run clear clean all static
all : ${PROGS}

test : ${PROGS}
	@for prog in ${PROGS}; do \
		printf "\033[01;34m%s\033[0m %s\n" "./$$prog" "$$prog.conf" 1>&2; \
		./$${prog} $${prog}.conf < /dev/null || break; \
		printf "\n"; \
	done

clear :
	rm -f ${PROGS} ${OUTPUT} *~
clean : clear
	rm -f ${OBJS}
