/*!
 * read basic and specific configuration from files.
 */

#include <stdio.h>

#include "node.h"

#include "parse.h"

/*!
 * \ingroup mptParse
 * \brief read config file
 * 
 * Try to get file parameters from generic value
 * and set node elements according to parsed content.
 * 
 * \param conf configuration target
 * \param val  input file properties
 * \param log  logging descriptor
 * 
 * \return string describing error
 */
extern int mpt_node_parse(MPT_STRUCT(node) *conf, const MPT_STRUCT(value) *val, MPT_INTERFACE(logger) *log)
{
	const char *fname, *format, *limit;
	FILE *fd;
	int res, len;
	
	if (!val) {
		return MPT_ERROR(BadArgument);
	}
	if (!val->fmt) {
		if (!(fname = val->ptr)) {
			return MPT_ERROR(BadArgument);
		}
		len = 1;
	}
	else if (val->fmt[0] == MPT_ENUM(TypeFile)) {
		FILE * const *ptr = val->ptr;
		fname = 0;
		if (!(fd = *ptr)) {
			return MPT_ERROR(BadValue);
		}
		len = 1;
	}
	else if (val->fmt[0] == 's') {
		char * const *ptr = val->ptr;
		if (!(fname = *ptr)) {
			return MPT_ERROR(BadValue);
		}
		len = 1;
	}
	else {
		return MPT_ERROR(BadType);
	}
	format = 0;
	limit = "ns";
	
	if (val->fmt[1]) {
		char * const *ptr = val->ptr;
		if (val->fmt[1] != 's') {
			if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s", MPT_tr("bad format description"));
			return MPT_ERROR(BadType);
		}
		format = ptr[1];
		len = 2;
		
		if (val->fmt[2]) {
			if (val->fmt[2] != 's') {
				if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s", MPT_tr("bad name limit"));
				return MPT_ERROR(BadType);
			}
			limit = ptr[2];
			len = 3;
		}
	}
	if (fname && !(fd = fopen(fname, "r"))) {
		if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s: %s", MPT_tr("failed to open file"), fname);
		return MPT_ERROR(BadValue);
	}
	res = mpt_node_read(conf, fd, format, limit, log);
	if (fname) {
		fclose(fd);
	}
	return res < 0 ? res : len;
}
