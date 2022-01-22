/*
 * MPT core library
 *   demonstrate value handling in pure C
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(convert.h)
#include MPT_INCLUDE(types.h)

ssize_t out(void *ctx, const char *str, size_t len)
{
	return fwrite(str, 1, len, ctx);
}

int main() {
	static const char text[] = "1234567890abcdefghijkl";
	MPT_STRUCT(value) val = MPT_VALUE_INIT(0, 0);
	struct iovec io;
	int32_t i = 23;
	double d = 23;
	long double e = 23;
	
	const char *nl = mpt_newline_string(0);
	
	MPT_value_set_string(&val, text);
	mpt_tostring(&val, out, stdout);
	fputs(nl, stdout);
	
	MPT_value_set_data(&val, 'i', &i);
	mpt_tostring(&val, out, stdout);
	fputs(nl, stdout);
	
	MPT_value_set_data(&val, 'd', &d);
	mpt_tostring(&val, out, stdout);
	fputs(nl, stdout);
	
	MPT_value_set_data(&val, 'e', &e);
	mpt_tostring(&val, out, stdout);
	fputs(nl, stdout);
	
	io.iov_base = &d;
	io.iov_len  = sizeof(d);
	MPT_value_set_data(&val, 'D', &io);
	mpt_tostring(&val, out, stdout);
	fputs(nl, stdout);
	
	return !(val._buf[0] != 0);
}
