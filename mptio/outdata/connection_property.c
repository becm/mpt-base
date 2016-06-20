/*!
 * set initial parameter for output descriptor
 */

#define _POSIX_C_SOURCE 1 /* for fdopen() */

#include <stdio.h>
#include <string.h>
#include <strings.h> /* for strcasecmp() */

#include "array.h"
#include "queue.h"
#include "event.h"

#include "convert.h"

#include "output.h"

static int setHistfile(FILE **hist, MPT_INTERFACE(metatype) *src)
{
	const char *where = 0;
	int len;
	FILE *fd;
	
	if (!src) {
		return *hist ? 1 : 0;
	}
	if ((len = src->_vptr->conv(src, 's', &where)) < 0) {
		return len;
	} else if (!where) {
		fd = stdout;
	} else if (!*where) {
		fd = 0;
	} else {
		MPT_STRUCT(socket) sock = MPT_SOCKET_INIT;
		int mode;
		
		/* try to use argument as connect string */
		if ((mode = mpt_connect(&sock, where, 0)) >= 0) {
			if (!(mode & MPT_ENUM(SocketStream))
			    || !(mode & MPT_ENUM(SocketWrite))
			    || !(fd = fdopen(sock._id, "w"))) {
				mpt_connect(&sock, 0, 0);
				return -1;
			}
		}
		/* regular file path */
		else if (!(fd = fopen(where, "w"))) {
			return -1;
		}
	}
	if (*hist && (*hist != stdout) && (*hist != stderr)) {
		fclose(*hist);
	}
	*hist = fd;
	
	return len;
}
static int outputEncoding(MPT_STRUCT(connection) *con, MPT_INTERFACE(metatype) *src)
{
	MPT_TYPE(DataEncoder) enc;
	MPT_TYPE(DataDecoder) dec;
	char *where;
	int32_t val;
	int type;
	uint8_t rtype;
	
	if (!src) return con->_coding;
	
	if (MPT_socket_active(&con->out.sock)) {
		return -4;
	}
	if ((type = src->_vptr->conv(src, 's', &where)) >= 0) {
		val = mpt_encoding_value(where, -1);
		if (val < 0 || val > UINT8_MAX) {
			return -2;
		}
		rtype = val;
	}
	else if ((type = src->_vptr->conv(src, 'y', &rtype)) < 0) {
		type = src->_vptr->conv(src, 'i', &val);
		if (type < 0 || val < 0 || val > UINT8_MAX) {
			return -3;
		}
		rtype = val;
	}
	if (!rtype) {
		enc = 0;
		dec = 0;
	}
	else if (!(enc = mpt_message_encoder(rtype))
	    || !(dec = mpt_message_decoder(rtype))) {
		return -3;
	}
	if (con->out._enc.fcn) {
		con->out._enc.fcn(&con->out._enc.info, 0, 0);
		con->out._enc.fcn = enc;
	}
	if (con->in._dec) {
		con->in._dec(&con->in._state, 0, 0);
		con->in._dec = dec;
	}
	con->_coding = rtype;
	
	return type;
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
	if (!name || !*name) {
		return mpt_outdata_set(&con->out, name, src);
	}
	if (!strcasecmp(name, "history") || !strcasecmp(name, "histfile")) {
		return setHistfile(&con->hist.file, src);
	}
	else if (!strcasecmp(name, "histfmt")) {
		return mpt_valfmt_set(&con->hist.info._fmt, src);
	}
	else if (!strcasecmp(name, "encoding")) {
		return outputEncoding(con, src);
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
		return mpt_outdata_get(&con->out, pr);
	}
	if (!*name) {
		static const char fmt[] = { MPT_ENUM(TypeSocket), 0 };
		
		pr->name = "connection";
		pr->desc = "interface to output data";
		pr->val.fmt = fmt;
		pr->val.ptr = &con->out.sock;
		
		return 0;
	}
	if (!strcasecmp(name, "history") || !strcasecmp(name, "histfile")) {
		pr->name = "history";
		pr->desc = MPT_tr("history data output file");
		pr->val.fmt = "";
		pr->val.ptr = con->hist.file;
		return con->hist.file ? 1 : 0;
	}
	if (!strcasecmp(name, "histfmt")) {
		MPT_STRUCT(buffer) *buf;
		pr->name = "histfmt";
		pr->desc = "history data output format";
		pr->val.fmt = "@";
		pr->val.ptr = &con->hist.info._fmt;
		buf = con->hist.info._fmt._buf;
		return buf ? buf->used : 0;
	}
	if (!strcasecmp(name, "encoding")) {
		pr->name = "encoding";
		pr->desc = "socket stream encoding";
		pr->val.fmt = "y";
		pr->val.ptr = &con->_coding;
		return con->_coding;
	}
	return mpt_outdata_get(&con->out, pr);
}
