/*!
 * read basic and specific configuration from files.
 */

#include <stdio.h>

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
extern int mpt_node_parse(MPT_STRUCT(node) *conf, FILE *file, const char *format, const char *limits, MPT_INTERFACE(logger) *log)
{
	MPT_STRUCT(parser_context) parse = MPT_PARSER_INIT;
	MPT_STRUCT(node) *old;
	int res;
	
	if (!file) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s", MPT_tr("bad file argument"));
		return MPT_ERROR(BadArgument);
	}
	if (!limits) {
		limits = "ns";
	}
	parse.src.getc = (int (*)()) mpt_getchar_stdio;
	parse.src.arg  = file;
	if ((res = mpt_parse_accept(&parse.name, limits)) < 0) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s: %s", MPT_tr("bad name limits"), limits);
		return MPT_ERROR(BadArgument);
	}
	
	/* clear children to trigger non-merge read */
	old = conf->children;
	conf->children = 0;
	
	res = mpt_parse_node(conf, &parse, format);
	/* replace tree content */
	if (res >= 0) {
		MPT_STRUCT(node) root = MPT_NODE_INIT;
		root.children = old;
		mpt_node_clear(&root);
		
		return res;
	}
	/* restore old subtree base */
	conf->children = old;
	
	if (log) {
		mpt_log(log, __func__, MPT_LOG(Error), "%s (%x): %s %u",
		        MPT_tr("parse error"), parse.curr, MPT_tr("line"), (int) parse.src.line);
	}
	return res;
}
