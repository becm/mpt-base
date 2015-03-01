# mpt.release.mk: create release information
release.h :
	@if [ -n "${MPT_RELEASE}" ]; then \
		echo "#define MPT_RELEASE \"${MPT_RELEASE}\"" > "${@}"; \
	else \
		echo "#define __MPT_DATE__ \"`date +%F`\"" > "${@}"; \
	fi
#
# add to auto-remove
CLEAN_FILES += release.h
