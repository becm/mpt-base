/*!
 * read basic and specific configuration from files.
 */

#include <stdio.h>

#include "meta.h"
#include "node.h"
#include "output.h"

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
	MPT_STRUCT(parser_context) parse = MPT_PARSER_INIT;
	MPT_STRUCT(node) *old;
	const char *fname, *format;
	int res, len;
	
	format = 0;
	
	parse.src.getc = (int (*)()) mpt_getchar_stdio;
	mpt_parse_accept(&parse.name, "ns");
	
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
	else {
		char * const *ptr = val->ptr;
		if (val->fmt[0] == MPT_ENUM(TypeFile)) {
			fname = 0;
			if (!(parse.src.arg = *ptr)) {
				if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s", MPT_tr("bad file argument"));
				return MPT_ERROR(BadValue);
			}
		}
		else if (val->fmt[0] == 's') {
			if (!(fname = *ptr)) {
				if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s", MPT_tr("bad file name part"));
				return MPT_ERROR(BadValue);
			}
		}
		else {
			return MPT_ERROR(BadType);
		}
		len = 1;
		/* additional format info */
		if (val->fmt[1]) {
			if (val->fmt[1] != 's') {
				if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s", MPT_tr("bad format description"));
				return MPT_ERROR(BadType);
			}
			format = ptr[1];
			len = 2;
			
			/* additional name limits */
			if (val->fmt[2]) {
				if (val->fmt[2] != 's') {
					if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s", MPT_tr("bad name limit"));
					return MPT_ERROR(BadType);
				}
				if ((res = mpt_parse_accept(&parse.name, ptr[2])) < 0) {
					if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s: %s", MPT_tr("bad name limit"), ptr[2]);
					return MPT_ERROR(BadValue);
				}
				len = 3;
			}
		}
	}
	if (fname) {
		if (!(parse.src.arg = fopen(fname, "r"))) {
			if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s: %s", MPT_tr("failed to open file"), fname);
			return MPT_ERROR(BadValue);
		}
	}
	/* clear children to trigger full read */
	old = conf->children;
	conf->children = 0;
	
	res = mpt_parse_node(conf, &parse, format);
	if (fname) {
		fclose(parse.src.arg);
	}
	/* replace tree content */
	if (res >= 0) {
		MPT_STRUCT(node) root = MPT_NODE_INIT;
		MPT_STRUCT(value) val = MPT_VALUE_INIT;
		root.children = old;
		mpt_node_clear(&root);
		
		if ((val.ptr = fname)) {
			mpt_meta_set(&conf->_meta, &val);
		}
		return len;
	}
	/* restore old subtree base */
	conf->children = old;
	
	if (!log) {
		return res;
	}
	if (fname) {
		(void) mpt_log(log, __func__, MPT_LOG(Error), "%s (%x): %s %u: %s",
		               MPT_tr("parse error"), parse.curr, MPT_tr("line"), (int) parse.src.line, fname);
	} else {
		(void) mpt_log(log, __func__, MPT_LOG(Error), "%s (%x): %s %u",
		               MPT_tr("parse error"), parse.curr, MPT_tr("line"), (int) parse.src.line);
	}
	return res;
}
