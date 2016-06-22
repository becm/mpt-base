/*!
 * set initial parameter for output descriptor
 */

#define _POSIX_C_SOURCE 1 /* for fdopen() */

#include <stdio.h>
#include <string.h>
#include <strings.h> /* for strcasecmp() */

#include <ctype.h> /* for isupper() */

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
		if (arg && isupper(*arg)) type |= MPT_ENUM(LogLevelFile);
		*level = type;
		return ret;
	}
	return MPT_ERROR(BadType);
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
	if (!strcasecmp(name, "history") || !strcasecmp(name, "histfile")) {
		return setHistfile(&con->hist.file, src);
	}
	else if (!strcasecmp(name, "histfmt")) {
		return mpt_valfmt_set(&con->hist.info._fmt, src);
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
			con->level = (MPT_ENUM(LogLevelInfo) << 4) | MPT_ENUM(LogLevelWarning);
		}
		else {
			con->level = ((v & 0xf) << 4) | (v & 0xf);
		}
		return ret;
	}
	if (!strcasecmp(name, "debug")) {
		uint8_t v = MPT_ENUM(LogLevelDebug3);
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
	if (!strcmp(name, "level") || !strcmp(name, "debug") || !strcasecmp(name, "answer")) {
		pr->name = "level";
		pr->desc = MPT_tr("output level");
		pr->val.fmt = "y";
		pr->val.ptr = &con->level;
		return con->level;
	}
	return mpt_outdata_get(&con->out, pr);
}
