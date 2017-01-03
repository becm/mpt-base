
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
	if (out->state & MPT_ENUM(OutputActive)) {
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

static int outdataColor(uint8_t *flags, MPT_INTERFACE(metatype) *src)
{
	char *where;
	int len;
	int32_t val = 0;
	
	if (!src) {
		*flags &= ~MPT_ENUM(OutputPrintColor);
		return 0;
	}
	if ((len = src->_vptr->conv(src, 's', &where)) >= 0) {
		if (!where || *where || !strcasecmp(where, "false")) {
			val = 1;
		} else {
			val = 0;
		}
	}
	else if ((len = src->_vptr->conv(src, 'i', &val)) < 0) {
		return MPT_ERROR(BadType);
	}
	if (val > 0) {
		*flags |= MPT_ENUM(OutputPrintColor);
	} else {
		*flags &= ~MPT_ENUM(OutputPrintColor);
	}
	return len;
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
	if (!strcasecmp(name, "color")) {
		return outdataColor(&od->state, src);
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
	intptr_t pos = -1, id;
	
	if (!pr) {
		return MPT_ENUM(TypeSocket);
	}
	/* find by position */
	if (!(name = pr->name)) {
		pos = (intptr_t) pr->desc;
	}
	else if (!*name) {
		static const char fmt[2] = { MPT_ENUM(TypeSocket), 0 };
		
		pr->name = "outdata";
		pr->desc = "output data context";
		pr->val.fmt = fmt;
		pr->val.ptr = &od->sock;
		
		return MPT_socket_active(&od->sock) ? 1 : 0;
	}
	id = 0;
	if (name ? !strcmp(name, "color") : pos == id++) {
		pr->name = "color";
		pr->desc = MPT_tr("colorized message output");
		pr->val.fmt = 0;
		if (od->state & MPT_ENUM(OutputPrintColor)) {
			pr->val.ptr = "true";
			return MPT_ENUM(OutputPrintColor);
		} else {
			pr->val.ptr = "false";
			return 0;
		}
	}
	return MPT_ERROR(BadArgument);
}

