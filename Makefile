# Makefile: create base MPT modules
MERGE = mptcore mptplot mptio mptloader
MODULES = ${MERGE} mpt++
SUB = ${MODULES} lua

include mpt.config.mk
#
# creation targets
.PHONY : ${SUB} all clear clean devel install static static++ sub_% examples_%
devel : sub_devel
install : sub_install
shared : sub_shared
test : examples_test
examples_test : devel
clear : examples_clear sub_clear
clean : examples_clean sub_clean
static : "${PREFIX_LIB}/libmpt.a" static++
mpt++ mptplot mptio : mptcore
mpt++ : mptplot mptio
lua : mptio
#
CLEAR_FILES += ${PREFIX_LIB}/libmpt.a
CLEAN_FILES += module_value.o module_generic_conv.o
# dispatch target to subdirectories
sub_% :
	@for m in ${SUB}; do \
		if ! ${MAKE} -C "$${m}" $(@:sub_%=%); then exit 1; fi; \
	done

${SUB} :
	${MAKE} -C "${@}"

static++ :
	${MAKE} -C "mpt++" static
#
# examples operations
examples_% :
	${MAKE} -C examples $(@:examples_%=%)
#
clear :
	${RM} ${CLEAR_FILES}

# combined static library
"${PREFIX_LIB}/libmpt.a" :
	@for m in ${MERGE}; do \
		if ! ${MAKE} -C "$${m}" static LIB=mpt "PREFIX_LIB=${PREFIX_LIB}"; then exit 1; fi; \
	done
	${AR} s "${PREFIX_LIB}/libmpt.a"
