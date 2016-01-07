/*!
 * insert elements from configuration file
 */

#include <stdio.h>
#include <errno.h>

#include "node.h"
#include "config.h"

#include "parse.h"

static int saveAppend(void *ctx, const MPT_STRUCT(path) *p, int last, int curr)
{
	MPT_STRUCT(node) *next, **pos = ctx;
	if ((next = mpt_node_append(*pos, p, last, curr))) {
		*pos = next;
		return 0;
	}
	return MPT_ERROR(BadOperation);
}
static int saveInsert(void *ctx, const MPT_STRUCT(path) *p, int last, int curr)
{
	MPT_STRUCT(node) *next, **base = ctx;
	(void) last;
	(void) curr;
	if (!(next = mpt_node_assign(base, p))) {
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
 * \param parse parse context
 * \param node  configuration target
 * 
 * \return parse result
 */
extern int mpt_parse_node(MPT_TYPE(ParserFcn) next, MPT_STRUCT(parse) *parse, MPT_STRUCT(node) *root)
{
	MPT_STRUCT(node) conf = MPT_NODE_INIT;
	int err;
	
	/* create new nodes */
	conf.children = 0;
	if (!(root->children)) {
		MPT_STRUCT(node) *curr = &conf;
		parse->lastop = MPT_ENUM(ParseSection);
		err = mpt_parse_config(next, parse, saveAppend, &curr);
	} else {
		parse->lastop = 0;
		err = mpt_parse_config(next, parse, saveInsert, &conf.children);
	}
	/* clear created nodes on error */
	if (err < 0) {
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
	return err;
}