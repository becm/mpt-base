# Makefile: create base MPT modules
MODULES = mptcore mptplot mptio mptloader mpt++
SUB = ${MODULES} lua

DIR_TOP=${MPT_PREFIX}
include mpt.config.mk
#
# creation targets
.PHONY : ${SUB} all clear clean devel install static static_clear sub_% examples_%
devel : sub_devel
install : sub_install
shared : sub_shared
test : examples_test
examples_test : install
clear : examples_clear sub_clear
clean : examples_clean sub_clean
static : "${DIR_LIB}/libmpt.a"
mpt++ mptplot mptio : mptcore
mpt++ : mptplot mptio
lua : mptio
#
# dispatch target to subdirectories
sub_% :
	@for m in ${SUB}; do \
		if ! ${MAKE} -C "$${m}" $(@:sub_%=%); then exit 1; fi; \
	done

${SUB} :
	${MAKE} -C "${@}"
#
# examples operations
examples_% :
	${MAKE} -C examples $(@:examples_%=%)
#
# combined static library
clear :
	${RM} "${DIR_LIB}/libmpt.a"

"${DIR_LIB}/libmpt.a" :
	@for m in ${MODULES}; do \
		if ! ${MAKE} -C "$${m}" static LIB=mpt "DIR_LIB=${DIR_LIB}"; then exit 1; fi; \
	done
	${AR} s "${DIR_LIB}/libmpt.a"
