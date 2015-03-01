# Makefile: create base MPT modules
MODULES = mptcore mpt++ mptplot mptclient lua examples
#
# creation targets
.PHONY : default ${MODULES}
default : ${MODULES}
mpt++ mptplot : mptcore
mpt++ : mptplot
${MODULES} : release.h
	@${MAKE} -C "${@}"
#
# release information
release.h :
	@if [ -n "${MPT_RELEASE}" ]; then \
		echo "#define MPT_RELEASE \"${MPT_RELEASE}\"" > release.h; \
	else \
		echo "#define __MPT_DATE__ \"`date +%F`\"" > release.h; \
	fi
#
# dispatch target to modules
.DEFAULT :
	@for m in ${MODULES}; do \
		if ! ${MAKE} -C "$${m}" ${@}; then break; fi; \
	done
#
