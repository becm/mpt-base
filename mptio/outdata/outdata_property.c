
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

static int outdataSocket(MPT_STRUCT(outdata) *out, MPT_INTERFACE(metatype) *src)
{
	
	MPT_STRUCT(socket) sock = MPT_SOCKET_INIT;
	int len;
	
	if (!src) {
		return MPT_ERROR(BadArgument);
	}
	if (out->state & MPT_OUTFLAG(Active)) {
		return MPT_ERROR(BadOperation);
	}
	if (src && (len = src->_vptr->conv(src, MPT_ENUM(TypeSocket), &sock)) >= 0) {
		int val;
		socklen_t valsz = sizeof(val);
		if (getsockopt(sock._id, SOL_SOCKET, SO_TYPE, &val, &valsz) < 0) {
			return MPT_ERROR(BadOperation);
		}
		return MPT_ERROR(BadValue);
	}
	return MPT_ERROR(BadType);
}

/*!
 * \ingroup mptOutput
 * \brief query outdata properties
 * 
 * Set/Get properties of outdata.
 * 
 * \param sock target output descriptor
 * \param name property to query
 * \param src  data source to change property
 */
extern int mpt_outdata_set(MPT_STRUCT(outdata) *od, const char *name, MPT_INTERFACE(metatype) *src)
{
	if (!name) {
		return src ? outdataSocket(od, src) : MPT_ENUM(TypeSocket);
	}
	/* cast operation */
	if (!*name) {
		return outdataSocket(od, src);
	}
	return MPT_ERROR(BadArgument);
}

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
	if (!(name = pr->name) && !*name) {
		static const char fmt[2] = { MPT_ENUM(TypeSocket), 0 };
		
		pr->name = "outdata";
		pr->desc = "output data context";
		pr->val.fmt = fmt;
		pr->val.ptr = &od->sock;
		
		return MPT_socket_active(&od->sock) ? 1 : 0;
	}
	return MPT_ERROR(BadArgument);
}

