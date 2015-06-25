# mpt.release.mk: create release information
RELEASE_HEADER ?= release.h
${RELEASE_HEADER} :
	@if [ -n "${MPT_RELEASE}" ]; then \
		echo "#define MPT_RELEASE \"${MPT_RELEASE}\"" > "${@}"; \
	else \
		echo "#define __MPT_DATE__ \"`date +%F`\"" > "${@}"; \
	fi
#
# add to auto-remove
GEN_FILES += ${RELEASE_HEADER}
