
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include <ctype.h>

#include "array.h"
#include "message.h"

#include "output.h"

static int outdataConnection(MPT_STRUCT(outdata) *out, MPT_INTERFACE(metatype) *src)
{
	char *where;
	int len;
	
	if (!src) {
		if (!MPT_socket_active(&out->sock)) {
			return 0;
		}
		if ((len = mpt_connect(&out->sock, 0, 0)) < 0) {
			return len;
		}
		/* TODO: cleanup if closed during activity */
		return len;
	}
	if (out->state & MPT_ENUM(OutputActive)) {
		return MPT_ERROR(BadOperation);
	}
	if ((len = src->_vptr->conv(src, 's', &where)) >= 0) {
		MPT_STRUCT(socket) tmp = MPT_SOCKET_INIT;
		int flg = 0;
		
		/* create new connection */
		if (where && (flg = mpt_connect(&tmp, where, 0)) < 0) {
			return MPT_ERROR(BadValue);
		}
		/* replace old descriptor */
		if (out->sock._id > 2) {
			(void) close(out->sock._id);
		}
		out->sock = tmp;
		out->_sflg = flg;
		return len;
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
/* set level limit */
static int outdataLevel(uint8_t *level, MPT_INTERFACE(metatype) *src)
{
	const char *arg;
	int ret;
	
	if (!src) {
		*level = 0;
		return 0;
	}
	if ((ret = src->_vptr->conv(src, 'k', &arg)) >= 0) {
		int type;
		if ((type = mpt_output_level(arg)) < 0) {
			return MPT_ERROR(BadValue);
		}
		if (arg && isupper(*arg)) type |= MPT_ENUM(OutputLevelLog);
		*level = type;
		return ret;
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
	int ret;
	
	if (!name) {
		return MPT_ENUM(TypeSocket);
	}
	/* cast operation */
	if (!*name) {
		return outdataConnection(od, src);
	}
	if (!strcasecmp(name, "color")) {
		return outdataColor(&od->state, src);
	}
	if (!strcasecmp(name, "level")) {
		uint8_t v = od->level;
		
		if (src && (ret = src->_vptr->conv(src, 'y', &v)) > 0) {
			od->level = v;
		}
		else if ((ret = outdataLevel(&v, src)) < 0) {
			return ret;
		}
		else if (!ret) {
			od->level = (MPT_ENUM(OutputLevelInfo) << 4) | MPT_ENUM(OutputLevelWarning);
		}
		else {
			od->level = ((v & 0xf) << 4) | (v & 0xf);
		}
		return ret;
	}
	if (!strcasecmp(name, "debug")) {
		uint8_t v = MPT_ENUM(OutputLevelDebug3);
		ret = 0;
		if (src && (ret = src->_vptr->conv(src, 'y', &v)) < 0) {
			return MPT_ERROR(BadType);
		}
		od->level = (od->level & 0xf0) | (v & 0xf);
		return ret;
	}
	if (!strcasecmp(name, "print")) {
		uint8_t v = od->level & 0xf;
		if ((ret = outdataLevel(&v, src)) < 0) {
			return ret;
		}
		od->level = (od->level & 0xf0) | (v & 0xf);
		return ret;
	}
	if (!strcasecmp(name, "answer")) {
		uint8_t v = (od->level & 0xf0) >> 4;
		if ((ret = outdataLevel(&v, src)) < 0) {
			return ret;
		}
		od->level = (od->level & 0xf) | ((v & 0xf) << 4);
		return ret;
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
	if (name ? (!strcmp(name, "level") || !strcmp(name, "debug") || !strcasecmp(name, "answer")) : pos == id++) {
		pr->name = "level";
		pr->desc = MPT_tr("output level");
		pr->val.fmt = "y";
		pr->val.ptr = &od->level;
		return od->level;
	}
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

