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
extern int mpt_config_read(MPT_STRUCT(node) *root, const char *file, const char *fmt, const char *limit, MPT_INTERFACE(logger) *out)
{
	MPT_STRUCT(parse) parse;
	MPT_STRUCT(node) conf;
	MPT_TYPE(ParserFcn) next;
	FILE *fd;
	int err;
	
	if (!file) {
		(void) mpt_log(out, __func__, MPT_ENUM(LogError), "%s", MPT_tr("no file specified"));
		errno = EFAULT; return -3;
	}
	if (!root) {
		(void) mpt_log(out, __func__, MPT_ENUM(LogError), "%s", MPT_tr("missing configuration node"));
		errno = EFAULT; return -3;
	}
	mpt_parse_init(&parse);
	err = mpt_parse_format(&parse.format, fmt);
	
	if (!(next = mpt_parse_next_fcn(err))) {
		(void) mpt_log(out, __func__, MPT_ENUM(LogError), "%s: \"%s\"", MPT_tr("invalid parse format"),  fmt ? fmt : "");
		return -2;
	}
	if ((err = mpt_parse_accept(&parse.name, limit)) < 0) {
		return err;
	}
	if (!(fd = fopen(file, "r"))) {
		(void) mpt_log(out, __func__, MPT_ENUM(LogError), "%s \"%s\"", MPT_tr("unable to open file"), file);
		return -1;
	}
	parse.src.getc = (int (*)()) mpt_getchar_stdio;
	parse.src.arg  = fd;
	
	conf.children = 0;
	if ((err = mpt_parse_config(next, &parse, &conf)) < 0) {
		mpt_node_clear(&conf);
	}
	else {
		MPT_STRUCT(node) *tmp;
		/* move non-present */
		if ((tmp = root->children)) {
			mpt_node_move(tmp, conf.children);
			mpt_node_clear(root);
		}
		/* reparent configuration list */
		root->children = tmp = conf.children;
		while (tmp) {
			tmp->parent = root;
			tmp = tmp->next;
		}
	}
	fclose(fd);
	if (err < 0) {
		mpt_log(out, __func__, MPT_ENUM(LogError), "%s (%x): %s %u: %s", MPT_tr("parse error"), -err, MPT_tr("line"), (int) parse.src.line, file);
	}
	return err;
}
