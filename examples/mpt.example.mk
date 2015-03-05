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
CLEAR_FILES ?= ${PROGS} ${TESTS} $(PROGS:%=%.out) $(TESTS:%=%.out)
CLEAN_FILES ?= $(OBJS)
#
# general rules
.PHONY: clear clean all static
all : ${PROGS}

test : ${TESTS}
	@for prog in ${TESTS}; do \
		printf "\033[01;34m%s\033[0m %s\n" "./$${prog}" "$${prog}.conf" 1>&2; \
		"./$${prog}" "$${prog}.conf" < /dev/null || break; \
		printf "\n"; \
	done

clear :
	rm -f ${CLEAR_FILES} *~
clean : clear
	rm -f ${CLEAN_FILES}

.PHONY : sub_% ${DIRS}
${DIRS} :
	${MAKE} -C "${@}"
sub_% :
	@for d in ${DIRS}; do if ! ${MAKE} -C "$${d}" $(@:sub_%=%); then break; fi; done;
