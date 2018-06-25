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
/*!
 * \ingroup mptParse
 * \brief parse config file
 * 
 * Parse config file with specific format description.
 * 
 * \param root  target for new elements
 * \param parse parse context
 * \param node  configuration target
 * 
 * \return parse result
 */
extern int mpt_parse_node(MPT_STRUCT(node) *root, MPT_STRUCT(parser_context) *parse, const char *fmt)
{
	MPT_STRUCT(parser_format) pfmt;
	MPT_TYPE(input_parser) next;
	int err;
	
	if (!root || !parse) {
		return MPT_ERROR(BadArgument);
	}
	err = mpt_parse_format(&pfmt, fmt);
	
	if (!(next = mpt_parse_next_fcn(err))) {
		return MPT_ERROR(BadType);
	}
	parse->prev = MPT_PARSEFLAG(Section);
	
	/* create new nodes */
	if (!(root->children)) {
		MPT_STRUCT(node) *curr = root;
		err = mpt_parse_config(next, &pfmt, parse, saveAppend, &curr);
		
		/* clear created nodes on error */
		if (err < 0) {
			mpt_node_clear(root);
		}
	}
	/* add to existing */
	else {
		MPT_STRUCT(node) conf = MPT_NODE_INIT;
		MPT_STRUCT(node) *curr = &conf;
		err = mpt_parse_config(next, &pfmt, parse, saveAppend, &curr);
		
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
