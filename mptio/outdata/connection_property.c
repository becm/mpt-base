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
		if (arg && isupper(*arg)) type |= MPT_LOG(LevelFile);
		*level = type;
		return ret;
	}
	return MPT_ERROR(BadType);
}

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
			return -2;
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
	if (!strcasecmp(name, "level")) {
		uint8_t v = con->level;
		
		if (src && (ret = src->_vptr->conv(src, 'y', &v)) > 0) {
			con->level = v;
		}
		else if ((ret = connectionLevel(&v, src)) < 0) {
			return ret;
		}
		else if (!ret) {
			con->level = (MPT_LOG(LevelInfo) << 4) | MPT_LOG(LevelWarning);
		}
		else {
			con->level = ((v & 0xf) << 4) | (v & 0xf);
		}
		return ret;
	}
	if (!strcasecmp(name, "debug")) {
		uint8_t v = MPT_LOG(LevelDebug3);
		ret = 0;
		if (src && (ret = src->_vptr->conv(src, 'y', &v)) < 0) {
			return MPT_ERROR(BadType);
		}
		con->level = (con->level & 0xf0) | (v & 0xf);
		return ret;
	}
	if (!strcasecmp(name, "print")) {
		uint8_t v = con->level & 0xf;
		if ((ret = connectionLevel(&v, src)) < 0) {
			return ret;
		}
		con->level = (con->level & 0xf0) | (v & 0xf);
		return ret;
	}
	if (!strcasecmp(name, "answer")) {
		uint8_t v = (con->level & 0xf0) >> 4;
		if ((ret = connectionLevel(&v, src)) < 0) {
			return ret;
		}
		con->level = (con->level & 0xf) | ((v & 0xf) << 4);
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
	
	if (!pr) {
		return MPT_ENUM(TypeOutput);
	}
	if (!(name = pr->name)) {
		if ((uintptr_t) pr->desc != 1) {
			return mpt_outdata_get(&con->out, pr);
		}
		name = "level";
	}
	else if (!*name) {
		static const char fmt[] = { MPT_ENUM(TypeSocket), 0 };
		
		pr->name = "connection";
		pr->desc = "interface to output data";
		pr->val.fmt = fmt;
		pr->val.ptr = &con->out.sock;
		
		return 0;
	}
	if (!strcmp(name, "level") || !strcmp(name, "debug") || !strcasecmp(name, "answer")) {
		pr->name = "level";
		pr->desc = MPT_tr("output level");
		pr->val.fmt = "y";
		pr->val.ptr = &con->level;
		return con->level;
	}
	return mpt_outdata_get(&con->out, pr);
}
