# Makefile: create base MPT modules
MODULES = mptcore mptplot mptio mptloader mpt++
SUB = ${MODULES} lua
#
# creation targets
.PHONY : ${SUB} all clear clean devel install static static_clear sub_% examples_%
devel : sub_devel
install : sub_install static
shared : sub_shared
test : examples_test
examples_test : devel
clear : examples_clear sub_clear
clean : examples_clean sub_clean
static : "${MPT_PREFIX_LIB}/libmpt.a"
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
	${RM} "${MPT_PREFIX_LIB}/libmpt.a"

"${MPT_PREFIX_LIB}/libmpt.a" :
	@for m in ${MODULES}; do \
		if ! ${MAKE} -C "$${m}" static LIB=mpt "DIR_LIB=${MPT_PREFIX_LIB}"; then exit 1; fi; \
	done
	${AR} s "${MPT_PREFIX_LIB}/libmpt.a"
