
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "queue.h"
#include "stream.h"

#include "array.h"
#include "message.h"

#include "meta.h"
#include "convert.h"

#include "output.h"

/*!
 * \ingroup mptOutput
 * \brief query outdata properties
 * 
 * Get properties of outdata.
 * 
 * \param od   output data descriptor
 * \param prop property data
 */
extern int mpt_outdata_get(const MPT_STRUCT(outdata) *od, MPT_STRUCT(property) *pr)
{
	const char *name;
	
	if (!pr) {
		return MPT_ENUM(TypeSocket);
	}
	if ((name = pr->name) && !*name) {
		static const char fmt[2] = { MPT_ENUM(TypeSocket), 0 };
		
		pr->name = "outdata";
		pr->desc = "output data context";
		pr->val.fmt = fmt;
		pr->val.ptr = &od->sock;
		
		return MPT_socket_active(&od->sock) ? 1 : 0;
	}
	return MPT_ERROR(BadArgument);
}
