
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
	MPT_STRUCT(socket) tmp = MPT_SOCKET_INIT;
	char *where;
	int len;
	
	if (!src) {
		return MPT_socket_active(&out->sock) ? out->sock._id + 1 : 0;
	}
	if (out->state & MPT_ENUM(OutputActive)) {
		return -4;
	}
	if ((len = src->_vptr->conv(src, 's', &where)) >= 0) {
		int flg;
		if (!where) {
			if (out->sock._id > 2) {
				(void) close(out->sock._id);
				out->sock._id = -1;
			}
			return 0;
		}
		if ((flg = mpt_connect(&tmp, where, 0)) < 0) {
			return -2;
		}
		out->_sflg = flg;
	}
	/* replace old descriptor */
	if (out->sock._id > 2) {
		(void) close(out->sock._id);
	}
	out->sock = tmp;
	
	return len;
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
		return -2;
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
		return ret;
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
	if ((ret = src->_vptr->conv(src, 's', &arg)) < 0) {
		return ret;
	}
	if (!arg || !*arg) {
		*level = MPT_ENUM(OutputLevelWarning);
	}
	else if (!strcasecmp(arg, "none")) {
		*level = MPT_ENUM(OutputLevelNone);
	}
	else if (!strcasecmp(arg, "fatal") || !strcasecmp(arg, "critical")) {
		*level = MPT_ENUM(OutputLevelCritical);
	}
	else if (!strcasecmp(arg, "error")) {
		*level = MPT_ENUM(OutputLevelError);
	}
	else if (!strcasecmp(arg, "warning") || !strcasecmp(arg, "default")) {
		*level = MPT_ENUM(OutputLevelWarning);
	}
	else if (!strcasecmp(arg, "info")) {
		*level = MPT_ENUM(OutputLevelInfo);
	}
	else if (!strcasecmp(arg, "debug") || !strcasecmp(arg, "debug1")) {
		*level = MPT_ENUM(OutputLevelDebug1);
	}
	else if (!strcasecmp(arg, "debug2")) {
		*level = MPT_ENUM(OutputLevelDebug2);
	}
	else if (!strcasecmp(arg, "debug3")) {
		*level = MPT_ENUM(OutputLevelDebug3);
	}
	else {
		return -2;
	}
	if (isupper(*arg)) {
		*level |= MPT_ENUM(OutputLevelLog);
	}
	return strlen(arg);
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
			return -1;
		}
		pos = (intptr_t) prop->desc;
	}
	else if (!*name) {
		static const char fmt[2] = { MPT_ENUM(TypeSocket) };
		if ((ret = outdataConnection(od, src)) < 0) {
			return -2;
		}
		prop->name = "outdata";
		prop->desc = "output data context";
		prop->fmt  = fmt;
		prop->data = &od->sock;
		return ret;
	}
	if (name ? !strcasecmp(name, "color") : pos == id++) {
		if ((ret = outdataColor(&od->state, src)) < 0) {
			return -2;
		}
		prop->name = "color";
		prop->desc = MPT_tr("colorized message output");
		prop->fmt  = 0;
		prop->data = od->state & MPT_ENUM(OutputPrintColor) ? "true" : "false";
		return ret;
	}
	if (name ? !strcasecmp(name, "level") : pos == id++) {
		uint8_t v = od->level;
		
		if (src && (ret = src->_vptr->conv(src, 'B', &v)) > 0) {
			od->level = v;
		}
		else if ((ret = outdataLevel(&v, src)) < 0) {
			return -2;
		}
		else if (!ret) {
			od->level = (MPT_ENUM(OutputLevelInfo) << 4) | MPT_ENUM(OutputLevelWarning);
		}
		else {
			od->level = ((v & 0xf) << 4) | (v & 0xf);
		}
		prop->name = "level";
		prop->desc = MPT_tr("output level");
		prop->fmt  = "B";
		prop->data = &od->level;
		return ret;
	}
	if (name && !strcasecmp(name, "debug")) {
		uint8_t v = od->level;
		if (!src) {
			return -3;
		}
		if ((ret = outdataDebug(&v, src)) < 0) {
			return -2;
		}
		od->level = (od->level & 0xf0) | (v & 0xf);
		prop->name = "level";
		prop->desc = MPT_tr("output level");
		prop->fmt  = "B";
		prop->data = &od->level;
		return ret;
	}
	if (name && !strcasecmp(name, "print")) {
		uint8_t v = od->level & 0xf;
		if ((ret = outdataLevel(&v, src)) < 0) {
			return -2;
		}
		od->level = (od->level & 0xf0) | (v & 0xf);
		prop->name = "level";
		prop->desc = MPT_tr("output level");
		prop->fmt  = "B";
		prop->data = &od->level;
		return ret;
	}
	else if (name && !strcasecmp(name, "answer")) {
		uint8_t v = (od->level & 0xf0) >> 4;
		if ((ret = outdataLevel(&v, src)) < 0) {
			return -2;
		}
		od->level = (od->level & 0xf) | ((v & 0xf) << 4);
		prop->name = "level";
		prop->desc = MPT_tr("output level");
		prop->fmt  = "B";
		prop->data = &od->level;
		return ret;
	}
	return -1;
}

