
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include <ctype.h>

#include "array.h"
#include "message.h"

#include "output.h"

static int outdataConnection(MPT_STRUCT(outdata) *out, MPT_INTERFACE(source) *src)
{
	char *where;
	int len;
	
	if (!src) {
		return MPT_socket_active(&out->sock) ? out->sock._id + 1 : 0;
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

static int outdataColor(uint8_t *flags, MPT_INTERFACE(source) *src)
{
	char *where;
	int len, val = 0;
	
	if (!src) {
		return (*flags & MPT_ENUM(OutputPrintColor)) ? 1 : 0;
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

static int outdataDebug(uint8_t *level, MPT_INTERFACE(source) *src)
{
	int ret;
	uint8_t val = MPT_ENUM(OutputLevelDebug3);
	if ((ret = src->_vptr->conv(src, 'B', &val)) < 0) {
		return MPT_ERROR(BadType);
	}
	*level = (*level & 0x8) | (val < 0x3 ? val + MPT_ENUM(OutputLevelInfo) : 0x7);
	return ret;
}

/* set level limit */
static int outdataLevel(uint8_t *level, MPT_INTERFACE(source) *src)
{
	const char *arg;
	int ret;
	
	if (!src) {
		return *level;
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
 * \param prop property to query
 * \param src  data source to change property
 */
extern int mpt_outdata_property(MPT_STRUCT(outdata) *od, MPT_STRUCT(property) *prop, MPT_INTERFACE(source) *src)
{
	const char *name;
	uintptr_t pos, id = 0;
	int ret;
	
	if (!prop) {
		return src ? outdataConnection(od, src) : MPT_ENUM(TypeSocket);
	}
	/* cast operation */
	if (!(name = prop->name)) {
		if (src) {
			return MPT_ERROR(BadArgument);
		}
		pos = (intptr_t) prop->desc;
	}
	else if (!*name) {
		static const char fmt[2] = { MPT_ENUM(TypeSocket) };
		if ((ret = outdataConnection(od, src)) < 0) {
			return ret;
		}
		prop->name = "outdata";
		prop->desc = "output data context";
		prop->val.fmt = fmt;
		prop->val.ptr = &od->sock;
		return ret;
	}
	if (name ? !strcasecmp(name, "color") : pos == id++) {
		if ((ret = outdataColor(&od->state, src)) < 0) {
			return ret;
		}
		prop->name = "color";
		prop->desc = MPT_tr("colorized message output");
		prop->val.fmt = 0;
		prop->val.ptr = od->state & MPT_ENUM(OutputPrintColor) ? "true" : "false";
		return ret;
	}
	if (name ? !strcasecmp(name, "level") : pos == id++) {
		uint8_t v = od->level;
		
		if (src && (ret = src->_vptr->conv(src, 'B', &v)) > 0) {
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
		prop->name = "level";
		prop->desc = MPT_tr("output level");
		prop->val.fmt = "B";
		prop->val.ptr = &od->level;
		return ret;
	}
	if (name && !strcasecmp(name, "debug")) {
		uint8_t v = od->level;
		if (!src) {
			return MPT_ERROR(BadOperation);
		}
		if ((ret = outdataDebug(&v, src)) < 0) {
			return ret;
		}
		od->level = (od->level & 0xf0) | (v & 0xf);
		prop->name = "level";
		prop->desc = MPT_tr("output level");
		prop->val.fmt = "B";
		prop->val.ptr = &od->level;
		return ret;
	}
	if (name && !strcasecmp(name, "print")) {
		uint8_t v = od->level & 0xf;
		if ((ret = outdataLevel(&v, src)) < 0) {
			return ret;
		}
		od->level = (od->level & 0xf0) | (v & 0xf);
		prop->name = "level";
		prop->desc = MPT_tr("output level");
		prop->val.fmt = "B";
		prop->val.ptr = &od->level;
		return ret;
	}
	else if (name && !strcasecmp(name, "answer")) {
		uint8_t v = (od->level & 0xf0) >> 4;
		if ((ret = outdataLevel(&v, src)) < 0) {
			return ret;
		}
		od->level = (od->level & 0xf) | ((v & 0xf) << 4);
		prop->name = "level";
		prop->desc = MPT_tr("output level");
		prop->val.fmt = "B";
		prop->val.ptr = &od->level;
		return ret;
	}
	return MPT_ERROR(BadArgument);
}

