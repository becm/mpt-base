# mpt.lib.mk: library creation template
#
# require library name
ifeq (${LIB},)
  $(error no library name defined)
endif
#
# missing shared lib version
SHLIB_MAJOR ?= 1
SHLIB_MINOR ?= 0
SHLIB_TEENY ?= 0
#
# linker and link options
LINK ?= ${CC}
LDDIRS ?= "${DIR_LIB}"
LDFLAGS = -shared ${CFLAGS} $(LDDIRS:%=-L%) '-Wl,-hlib${LIB}.so.${SHLIB_MAJOR}' -Wl,-rpath=\$$ORIGIN
#
OBJS ?= $(SRCS:%.c=%.o)
#
# include global configuration
include $(dir $(lastword $(MAKEFILE_LIST)))mpt.conf.mk
#
# path to library without type suffix
LIB_FULLNAME = ${DIR_LIB}/lib${LIB}
#
# general library rules
.PHONY: shared devel static header
devel  : ${LIB_FULLNAME}.so header
shared : ${LIB_FULLNAME}.so.${SHLIB_MAJOR}
static : ${LIB_FULLNAME}.a

${LIB_FULLNAME}.a : ${OBJS} ${KEEP_OBJS} ${STATIC_OBJS} ${LIB_FULLNAME}.a(${OBJS} ${KEEP_OBJS} ${STATIC_OBJS})
	${AR} s ${LIB_FULLNAME}.a

${LIB_FULLNAME}.a(%.o) : %.o
	${AR} S${ARFLAGS} '${@}' $?

${LIB_FULLNAME}.so : ${LIB_FULLNAME}.so.${SHLIB_MAJOR}
	cd ${@D}; ln -fs '${@F}.${SHLIB_MAJOR}' '${@F}'

${LIB_FULLNAME}.so.${SHLIB_MAJOR} : ${LIB_FULLNAME}.so.${SHLIB_MAJOR}.${SHLIB_MINOR}.${SHLIB_TEENY}
	cd ${@D}; ln -fs '${@F}.${SHLIB_MINOR}.${SHLIB_TEENY}' '${@F}'

${LIB_FULLNAME}.so.${SHLIB_MAJOR}.${SHLIB_MINOR}.${SHLIB_TEENY} : ${OBJS} ${KEEP_OBJS} ${SHLIB_OBJS}
	${LINK} ${LDFLAGS} -o '${@}' ${OBJS} ${KEEP_OBJS} ${SHLIB_OBJS} ${LDLIBS}
#
# header export
header : ${HEADER}
	if [ -n "${HEADER}" ]; then ${INST.d} '${DIR_INC}'; ${INST.h} ${HEADER}; fi
#
# maintenance targets
extensions = a so so.${SHLIB_MAJOR} so.${SHLIB_MAJOR}.${SHLIB_MINOR} so.${SHLIB_MAJOR}.${SHLIB_MINOR}.${SHLIB_TEENY}
CLEAR_FILES += $(extensions:%=${LIB_FULLNAME}.%)
.PHONY: clear clean distclean
clear :
	${RM} ${CLEAR_FILES}

clean : clear
	${RM} ${OBJS} ${SHLIB_OBJS} ${STATIC_OBJS} ${CLEAN_FILES}

distclean : clean

# additional dependencies
${OBJS} ${SHLIB_OBJS} : ${HEADER}
