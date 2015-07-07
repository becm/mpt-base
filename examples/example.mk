# example.mk: template for examples
#
# top directory
DIR_TOP ?= $(dir $(lastword $(MAKEFILE_LIST)))..
#
# flags for compiler/preprocessor
CPPFLAGS ?= -I${DIR_TOP}/include -Wall -Werror -Wpedantic
CFLAGS   ?= -g -fstack-protector
CXXFLAGS ?= ${CFLAGS} ${CPPFLAGS}
FFLAGS   ?= ${CFLAGS} ${CPPFLAGS}
#
# flags for linker
LDDIRS  ?= ${MPT_PREFIX_LIB}
DLDIRS  ?= ${LDDIRS}
LDFLAGS ?= $(LDDIRS:%=-L%) $(DLDIRS:%=-Wl,-R%)
LINK    ?= ${CC}
#
# static version of all programs
STATIC ?= $(PROGS:%=%_static)
#
# auto-generated content
CLEAR_FILES ?= ${PROGS} ${STATIC}
CLEAN_FILES ?= ${OBJS}
#
# general rules
.PHONY: clear clean all static sub_% test_%
all : sub_all ${PROGS}
test : sub_test $(TESTS:%=test_%)

test_% : %
	@prog=$(@:test_%=%) ; \
	printf "\033[01;34m%s\033[0m " "./$${prog}" 1>&2; echo "${ARGS}" 1>&2; \
	"./$${prog}" ${ARGS} < /dev/null && printf "\n"

clear : sub_clear
	${RM} ${CLEAR_FILES}
clean : sub_clean
	${RM} ${CLEAN_FILES}
distclean : sub_distclean
	${RM} ${CLEAR_FILES} ${CLEAN_FILES}

# subdirectory template
sub_% :
	@for d in ${DIRS}; do if ! ${MAKE} -C "$${d}" $(@:sub_%=%); then break; fi; done;

# static template
static : ${STATIC} sub_static
%_static : %.cpp
	${CXX} -static ${CXXFLAGS} ${LDFLAGS} -o ${@} $^ ${LDLIBS}
%_static : %.c
	${CC} -static ${CPPFLAGS} ${CFLAGS} ${LDFLAGS} -o ${@} $^ ${LDLIBS}
