/*!
 * insert elements from configuration file
 */

#include <stdio.h>
#include <errno.h>

#include "node.h"
#include "config.h"

#include "parse.h"

static int saveAppend(void *ctx, const MPT_STRUCT(path) *p, const MPT_STRUCT(value) *val, int last, int curr)
{
	MPT_STRUCT(node) *next, **pos = ctx;
	if ((next = mpt_node_append(*pos, p, val, last, curr))) {
		*pos = next;
		return 0;
	}
	return MPT_ERROR(BadOperation);
}
static int saveInsert(void *ctx, const MPT_STRUCT(path) *p, const MPT_STRUCT(value) *val, int last, int curr)
{
	MPT_STRUCT(node) *next, **base = ctx;
	(void) last;
	
	/* no data operation */
	if (curr == MPT_PARSEFLAG(SectEnd)) {
		return 0;
	}
	if (!(next = mpt_node_assign(base, p, val))) {
		return MPT_ERROR(BadOperation);
	}
	return 0;
}
/*!
 * \ingroup mptConfig
 * \brief parse config file
 * 
 * Parse config file with specific format description.
 * 
 * \param next  function to get next element
 * \param npar  context for 'next' function
 * \param parse parse context
 * \param node  configuration target
 * 
 * \return parse result
 */
extern int mpt_parse_node(MPT_STRUCT(node) *root, MPT_STRUCT(parse) *parse, const char *fmt)
{
	MPT_STRUCT(parsefmt) pfmt;
	MPT_TYPE(ParserFcn) next;
	int err;
	
	if (!root || !parse) {
		return MPT_ERROR(BadArgument);
	}
	err = mpt_parse_format(&pfmt, fmt);
	
	if (!(next = mpt_parse_next_fcn(err))) {
		return MPT_ERROR(BadType);
	}
	/* create new nodes */
	if (!(root->children)) {
		MPT_STRUCT(node) *curr = root;
		parse->prev = MPT_PARSEFLAG(Section);
		err = mpt_parse_config(next, &pfmt, parse, saveAppend, &curr);
		
		/* clear created nodes on error */
		if (err < 0) {
			mpt_node_clear(root);
		}
	}
	/* add to existing */
	else {
		MPT_STRUCT(node) conf = MPT_NODE_INIT;
		parse->prev = 0;
		err = mpt_parse_config(next, &pfmt, parse, saveInsert, &conf.children);
		
		/* clear created nodes on error */
		if (err < 0) {
			mpt_node_clear(&conf);
		}
		/* move non-present */
		else if (conf.children) {
			MPT_STRUCT(node) *curr;
			mpt_node_move(&root->children, conf.children);
			mpt_node_clear(root);
			root->children = curr = conf.children;
			/* set parent for moved nodes */
			while (curr) {
				curr->parent = root;
				curr = curr->next;
			}
		}
	}
	return err;
}
