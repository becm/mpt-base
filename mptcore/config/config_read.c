/*!
 * insert elements from configuration file
 */

#include <stdio.h>
#include <errno.h>

#include "node.h"
#include "parse.h"

#include "config.h"

/*!
 * \ingroup mptConfig
 * \brief read config file
 * 
 * Read config file with specific format description.
 * 
 * \param node contifguration node target
 * \param file file to parse
 * \param fmt  format of input file
 * \param out  error log descriptor
 * 
 * \return parse result
 */
extern int mpt_config_read(MPT_STRUCT(node) *root, FILE *fd, const char *fmt, const char *limit, MPT_INTERFACE(logger) *out)
{
	MPT_STRUCT(parse) parse;
	MPT_STRUCT(parsefmt) pfmt;
	MPT_TYPE(ParserFcn) next;
	int err;
	
	if (!root) {
		(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s",
		               MPT_tr("missing configuration node"));
		return MPT_ERROR(BadArgument);
	}
	if (!fd) {
		(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s",
		               MPT_tr("missing input file"));
		return MPT_ERROR(BadArgument);
	}
	mpt_parse_init(&parse);
	err = mpt_parse_format(&pfmt, fmt);
	
	if (!(next = mpt_parse_next_fcn(err))) {
		(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s: \"%s\"",
		               MPT_tr("invalid parse format"),  fmt ? fmt : "");
		return -2;
	}
	if ((err = mpt_parse_accept(&parse.name, limit)) < 0) {
		return err;
	}
	parse.src.getc = (int (*)()) mpt_getchar_stdio;
	parse.src.arg  = fd;
	
	/* replace children with parsed data */
	if ((err = mpt_parse_node(next, &pfmt, &parse, root)) < 0) {
		const char *fname;
		if ((fname = mpt_node_data(root, 0))) {
			(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s (%x): %s %u: %s",
			               MPT_tr("parse error"), parse.curr, MPT_tr("line"), (int) parse.src.line, fname);
		} else {
			(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s (%x): %s %u",
			               MPT_tr("parse error"), parse.curr, MPT_tr("line"), (int) parse.src.line);
		}
	}
	return err;
}
