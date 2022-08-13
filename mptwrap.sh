#!/bin/sh
# start target platform executable
#
# shellcheck shell=sh enable=all disable=SC2154
lib="${MPT_PREFIX_LIB}"
prog="$(basename "${0}")"

# select library destination
if [ -z "${lib}" ]; then \
  if [ -z "${MPT_PREFIX}" ]; then \
    lib=$(dirname "${0}")
    MPT_PREFIX=$(dirname "${lib}")
    export MPT_PREFIX
  fi
  lib="${MPT_PREFIX}/lib/$(arch)-$(uname -s | tr '[:upper:]' '[:lower:]')"
  [ -d "${lib}" ] || lib="${MPT_PREFIX}/lib"
fi
# set executable name
prog="${lib}/${prog}"

# call target executable
exec "${prog}" "$@"
