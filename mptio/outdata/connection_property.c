/*!
 * set initial parameter for output descriptor
 */

#define _POSIX_C_SOURCE 1 /* for fdopen() */

#include <stdio.h>
#include <string.h>
#include <strings.h> /* for strcasecmp() */

#include <ctype.h> /* for isupper() */

#include "array.h"
#include "event.h"

#include "convert.h"
#include "meta.h"

#include "stream.h"

#include "output.h"

static int connectionEncoding(MPT_STRUCT(connection) *con, MPT_INTERFACE(metatype) *src)
{
	MPT_TYPE(DataEncoder) enc;
	MPT_TYPE(DataDecoder) dec;
	MPT_STRUCT(stream) *srm;
	char *where;
	int32_t val;
	int res;
	uint8_t rtype;
	
	if (MPT_socket_active(&con->out.sock)) {
		if (src) {
			return MPT_ERROR(BadOperation);
		}
		return 0;
	}
	srm = (void *) con->out.buf._buf;
	if (!src) {
		if (!srm) {
			return MPT_ERROR(BadOperation);
		}
		rtype = MPT_ENUM(EncodingCobs);
	}
	else if ((res = src->_vptr->conv(src, 's', &where)) >= 0) {
		val = mpt_encoding_value(where, -1);
		if (val < 0 || val > UINT8_MAX) {
			return MPT_ERROR(BadValue);
		}
		rtype = val;
	}
	else if ((res = src->_vptr->conv(src, 'y', &rtype)) < 0) {
		res = src->_vptr->conv(src, 'i', &val);
		if (res < 0 || val < 0 || val > UINT8_MAX) {
			return MPT_ERROR(BadValue);
		}
		rtype = val;
	}
	/* active stream needs valid coding */
	if (!rtype
	    || !(enc = mpt_message_encoder(rtype))
	    || !(dec = mpt_message_decoder(rtype))) {
		return MPT_ERROR(BadValue);
	}
	if (srm->_wd._enc && (enc != srm->_wd._enc)) {
		if (srm->_wd.data.max) {
			return MPT_ERROR(BadOperation);
		}
		srm->_wd._enc(&srm->_wd._state, 0, 0);
	}
	if (srm->_rd._dec && (dec != srm->_rd._dec)) {
		if (srm->_rd.data.max) {
			return MPT_ERROR(BadOperation);
		}
		srm->_rd._dec(&srm->_rd._state, 0, 0);
	}
	srm->_wd._enc = enc;
	srm->_rd._dec = dec;
	
	return res;
}

/* set level limit */
static int connectionLevel(uint8_t *level, MPT_INTERFACE(metatype) *src)
{
	const char *arg;
	int ret;
	
	if (!src) {
		*level = 0;
		return 0;
	}
	if ((ret = src->_vptr->conv(src, 'k', &arg)) >= 0) {
		int type;
		if ((type = mpt_log_level(arg)) < 0) {
			return MPT_ERROR(BadValue);
		}
		*level = type;
		return ret;
	}
	return MPT_ERROR(BadType);
}
/* modify output color flag */
static int connectionColor(uint8_t *flags, MPT_INTERFACE(metatype) *src)
{
	char *where;
	int len;
	int32_t val = 0;
	
	if (!src) {
		*flags &= ~MPT_OUTFLAG(PrintColor);
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
		*flags |= MPT_OUTFLAG(PrintColor);
	} else {
		*flags &= ~MPT_OUTFLAG(PrintColor);
	}
	return len;
}
/*!
 * \ingroup mptOutput
 * \brief set connection property
 * 
 * Set property of connection or included outdata.
 * 
 * \param con  connection descriptor
 * \param name property to change
 * \param src  value source
 * 
 * \return state of property
 */
extern int mpt_connection_set(MPT_STRUCT(connection) *con, const char *name, MPT_INTERFACE(metatype) *src)
{
	int ret;
	if (!name || !*name) {
		if ((ret = mpt_outdata_set(&con->out, name, src)) >= 0) {
			/* clear waiting commands on connection */
			mpt_command_clear(&con->_wait);
		}
		return ret;
	}
	if (!strcasecmp(name, "encoding")) {
		return connectionEncoding(con, src);
	}
	if (!strcasecmp(name, "color")) {
		return connectionColor(&con->out.state, src);
	}
	if (!strcasecmp(name, "pass")) {
		uint8_t v = 0;
		if ((ret = connectionLevel(&v, src)) < 0) {
			return ret;
		}
		con->pass = v;
		return ret;
	}
	if (!strcasecmp(name, "show")) {
		uint8_t v = 0;
		if ((ret = connectionLevel(&v, src)) < 0) {
			return ret;
		}
		con->show = v;
		return ret;
	}
	return mpt_outdata_set(&con->out, name, src);
}

/*!
 * \ingroup mptOutput
 * \brief get connection property
 * 
 * Get property for connection or included outdata.
 * 
 * \param con  connection descriptor
 * \param pr   property to query
 * 
 * \return state of property
 */
extern int mpt_connection_get(const MPT_STRUCT(connection) *con, MPT_STRUCT(property) *pr)
{
	const char *name;
	intptr_t pos = -1, id;
	
	if (!pr) {
		return MPT_ENUM(TypeOutput);
	}
	if (!(name = pr->name)) {
		pos = (intptr_t) pr->desc;
	}
	else if (!*name) {
		pr->name = "connection";
		pr->desc = "interface to output data";
		/* socket is active */
		if (MPT_socket_active(&con->out.sock)) {
			static const char fmt[] = { MPT_ENUM(TypeSocket), 0 };
			pr->val.fmt = fmt;
			pr->val.ptr = &con->out.sock;
			return 1;
		}
		if (con->out.buf._buf) {
			pr->val.fmt = "";
			pr->val.ptr = con->out.buf._buf;
			return 1;
		}
		return 0;
	}
	id = 0;
	
	if (name ? (!strcasecmp(name, "pass") || !strcasecmp(name, "limit")) : pos == id++) {
		pr->name = "pass";
		pr->desc = MPT_tr("message type transfer limit");
		pr->val.fmt = "y";
		pr->val.ptr = &con->pass;
		return 1;
	}
	if (name ? (!strcasecmp(name, "show") || !strcasecmp(name, "debug")) : pos == id++) {
		pr->name = "show";
		pr->desc = MPT_tr("reply output level");
		pr->val.fmt = "y";
		pr->val.ptr = &con->show;
		return con->show ? 1 : 0;
	}
	if (name ? !strcmp(name, "color") : pos == id++) {
		pr->name = "color";
		pr->desc = MPT_tr("colorized message output");
		pr->val.fmt = 0;
		if (con->out.state & MPT_OUTFLAG(PrintColor)) {
			pr->val.ptr = "true";
			return 1;
		} else {
			pr->val.ptr = "false";
			return 0;
		}
	}
	return mpt_outdata_get(&con->out, pr);
}
