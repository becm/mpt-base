/*!
 * set initial parameter for output descriptor
 */

#include <stdio.h>
#include <string.h>
#include <strings.h> /* for strcasecmp() */

#include <ctype.h> /* for isupper() */

#include "meta.h"
#include "object.h"
#include "convert.h"
#include "message.h"

#include "stream.h"

#include "output.h"


static int connectionSet(MPT_STRUCT(connection) *con, const MPT_INTERFACE(metatype) *src)
{
	MPT_STRUCT(socket) sock = MPT_SOCKET_INIT;
	const char *where = 0;
	int ret;
	
	if (!src) {
		return MPT_ERROR(BadArgument);
	}
	if (con->out.state & MPT_OUTFLAG(Active)) {
		return MPT_ERROR(BadOperation);
	}
	if ((ret = src->_vptr->conv(src, MPT_ENUM(TypeSocket), &sock)) >= 0
	    && (ret = mpt_connection_assign(con, &sock)) >= 0) {
		return 0;
	}
	if ((ret = src->_vptr->conv(src, 's', &where)) < 0) {
		return ret;
	}
	if (!where) {
		mpt_connection_close(con);
		return 0;
	}
	if ((ret = mpt_connection_open(con, where, 0)) < 0) {
		return ret;
	}
	return 0;
}
/* set encoding for stream */
static int connectionEncoding(MPT_STRUCT(connection) *con, const MPT_INTERFACE(metatype) *src)
{
	MPT_TYPE(DataEncoder) enc;
	MPT_TYPE(DataDecoder) dec;
	MPT_STRUCT(stream) *srm = 0;
	char *where;
	int32_t val;
	int res;
	uint8_t rtype;
	
	if (!MPT_socket_active(&con->out.sock)) {
		srm = (void *) con->out.buf._buf;
	}
	/* (re)set default encoding */
	if (!src) {
		if (!srm) {
			return 0;
		}
		rtype = MPT_ENUM(EncodingCobs);
		res = 0;
	}
	/* resolve type description */
	else if ((res = src->_vptr->conv(src, 's', &where)) >= 0) {
		val = mpt_encoding_value(where, -1);
		if (val < 0 || val > UINT8_MAX) {
			return MPT_ERROR(BadValue);
		}
		rtype = val;
	}
	/* explicit encoding code */
	else if ((res = src->_vptr->conv(src, 'y', &rtype)) < 0) {
		res = src->_vptr->conv(src, 'i', &val);
		if (res < 0 || val < 0 || val > UINT8_MAX) {
			return MPT_ERROR(BadValue);
		}
		rtype = val;
	}
	/* no encoding on datagrams */
	if (!srm) {
		if (rtype) {
			return MPT_ERROR(BadValue);
		}
		return 0;
	}
	/* active stream needs valid coding */
	if (!rtype
	    || !(enc = mpt_message_encoder(rtype))
	    || !(dec = mpt_message_decoder(rtype))) {
		return MPT_ERROR(BadValue);
	}
	/* check if state is consistent */
	if (srm->_rd._state.scratch) {
		return MPT_MESGERR(ActiveInput);
	}
	val = mpt_stream_flags(&srm->_info);
	if (val & MPT_STREAMFLAG(MesgActive)) {
		return MPT_MESGERR(InProgress);
	}
	/* clear existing coding state contexts */
	if (srm->_rd._dec) {
		srm->_rd._dec(&srm->_rd._state, 0, 0);
	}
	if (srm->_wd._enc) {
		srm->_wd._enc(&srm->_wd._state, 0, 0);
	}
	srm->_wd._enc = enc;
	srm->_rd._dec = dec;
	
	return res;
}
/* modify output color flag */
static int connectionColor(uint8_t *flags, const MPT_INTERFACE(metatype) *src)
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
extern int mpt_connection_set(MPT_STRUCT(connection) *con, const char *name, const MPT_INTERFACE(metatype) *src)
{
	int ret;
	if (!name || !*name) {
		if ((ret = connectionSet(con, src)) >= 0) {
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
	return MPT_ERROR(BadArgument);
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
			static const uint8_t fmt[] = { MPT_ENUM(TypeSocket), 0 };
			pr->val.fmt = fmt;
			pr->val.ptr = &con->out.sock;
			return 1;
		}
		if (con->out.buf._buf) {
			pr->val.fmt = (uint8_t *) "";
			pr->val.ptr = con->out.buf._buf;
			return 1;
		}
		return 0;
	}
	id = 0;
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
