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
	
	format = 0;
	limit = "ns";
	
	if (!val) {
		if (!(fname = mpt_node_data(conf, 0))) {
			if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s", MPT_tr("no default filename"));
			return MPT_ERROR(BadArgument);
		}
		len = 0;
	}
	else if (!val->fmt) {
		if (!(fname = val->ptr)) {
			if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s", MPT_tr("bad file name argument"));
			return MPT_ERROR(BadArgument);
		}
		len = 1;
	}
	else if (val->fmt[0] == MPT_ENUM(TypeFile)) {
		FILE * const *ptr = val->ptr;
		fname = 0;
		if (!(fd = *ptr)) {
			if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s", MPT_tr("bad file argument"));
			return MPT_ERROR(BadValue);
		}
		len = 1;
	}
	else if (val->fmt[0] == 's') {
		char * const *ptr = val->ptr;
		if (!(fname = *ptr)) {
			if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s", MPT_tr("bad file name part"));
			return MPT_ERROR(BadValue);
		}
		len = 1;
		
		if (val->fmt[1]) {
			if (val->fmt[1] != 's') {
				if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s", MPT_tr("bad format description"));
				return MPT_ERROR(BadType);
			}
			format = ptr[1];
			len = 2;
			
			if (val->fmt[2]) {
				if (val->fmt[2] != 's') {
					if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s", MPT_tr("bad name limit"));
					return MPT_ERROR(BadType);
				}
				limit = ptr[2];
				len = 3;
			}
		}
	}
	else {
		return MPT_ERROR(BadType);
	}
	
	if (fname && !(fd = fopen(fname, "r"))) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s: %s", MPT_tr("failed to open file"), fname);
		return MPT_ERROR(BadValue);
	}
	res = mpt_node_read(conf, fd, format, limit, log);
	if (fname) {
		fclose(fd);
	}
	return res < 0 ? res : len;
}
