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
	
	/* no data operation */
	if (curr == MPT_ENUM(ParseSectEnd)) {
		return 0;
	}
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
extern int mpt_parse_node(MPT_TYPE(ParserFcn) next, void *npar, MPT_STRUCT(parse) *parse, MPT_STRUCT(node) *root)
{
	int err;
	
	/* create new nodes */
	if (!(root->children)) {
		MPT_STRUCT(node) *curr = root;
		parse->prev = MPT_ENUM(ParseSection);
		err = mpt_parse_config(next, npar, parse, saveAppend, &curr);
		
		/* clear created nodes on error */
		if (err < 0) {
			mpt_node_clear(root);
		}
	}
	/* add to existing */
	else {
		MPT_STRUCT(node) conf = MPT_NODE_INIT;
		parse->prev = 0;
		err = mpt_parse_config(next, npar, parse, saveInsert, &conf.children);
		
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
