
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "queue.h"
#include "stream.h"

#include "array.h"
#include "message.h"

#include "convert.h"

#include "output.h"

/*!
 * \ingroup mptOutput
 * \brief set outdata backend
 * 
 * Open target stream/socket and assign to output data.
 * 
 * \param out  output data descriptor
 * \param to   stream/socket target
 * \param fmt  connection type decsription
 */
extern int mpt_outdata_open(MPT_STRUCT(outdata) *out, const char *to, const char *fmt)
{
	MPT_STRUCT(fdmode) mode;
	int flg;
	
	if (fmt) {
		if ((flg = mpt_mode_parse(&mode, fmt)) < 0) {
			return MPT_ERROR(BadValue);
		}
	}
	else if ((flg = mpt_mode_parse(&mode, to)) < 0) {
		return mpt_outdata_connect(out, to, 0);
	}
	return mpt_outdata_connect(out, to+flg, &mode);
}
