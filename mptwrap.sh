#!/bin/sh
# start target platform executable
#
# shellcheck shell=sh enable=all
prog="$(basename "${0}")"
mach="$(cpp -dumpmachine)"

# resolve base directory
if [ -z "${MPT_PREFIX}" ]; then \
  bin="$(dirname "${0}")"
  [ "${bin}" = '.' ] && bin="$(pwd)"
  MPT_PREFIX=$(dirname "${bin}")
  export MPT_PREFIX
fi
# resolve lib directory
if [ -z "${MPT_PREFIX_LIB}" ]; then \
  MPT_PREFIX_LIB="${MPT_PREFIX}/lib/${mach}"
  if [ -d "${MPT_PREFIX_LIB}" ]; then
    export MPT_PREFIX_LIB
  else
    MPT_PREFIX_LIB="${MPT_PREFIX}/lib"
  fi
fi
# resolve platform executable
if [ -z "${MPT_PREFIX_LIBEXEC}" ]; then \
  MPT_PREFIX_LIBEXEC="${MPT_PREFIX}/libexec"
  if [ -x "${MPT_PREFIX_LIBEXEC}/${prog}" ]; then \
    export MPT_PREFIX_LIBEXEC
  elif [ -x "${MPT_PREFIX_LIB}/mpt/${prog}" ]; then \
    MPT_PREFIX_LIBEXEC="${MPT_PREFIX_LIB}/mpt"
    export MPT_PREFIX_LIBEXEC
  else
    MPT_PREFIX_LIBEXEC="${MPT_PREFIX_LIB}"
  fi
fi

# call platform executable
exec "${MPT_PREFIX_LIBEXEC}/${prog}" "$@"
