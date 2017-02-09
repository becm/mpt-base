
#include <errno.h>

#include "node.h"
#include "config.h"

#include "parse.h"

/*!
 * \ingroup mptParse
 * \brief add config element
 * 
 * append parsed element to configuration.
 * 
 * \param old    current configuration element
 * \param dest   path data of current element
 * \param prevop previous append operation
 * \param currop current append operation
 * 
 * \return created configuration element
 */
extern MPT_STRUCT(node) *mpt_node_append(MPT_STRUCT(node) *old, const MPT_STRUCT(path) *dest, int prevop, int currop)
{
	MPT_STRUCT(path) path = *dest;
	MPT_STRUCT(node) *conf;
	const char *data;
	
	/* no save operation */
	if (!(currop & 0xf)) {
		return old;
	}
	/* one level up */
	if (currop == MPT_PARSEFLAG(SectEnd)) {
		/* last operation was non-empty section */
		if (old && (prevop & MPT_PARSEFLAG(SectEnd))) {
			return old->parent;
		}
		return old;
	}
	/* new node has identifier */
	if (currop & MPT_PARSEFLAG(Section)) {
		if (mpt_path_last(&path) < 0) {
			return 0;
		}
		data = path.base + path.off;
	}
	/* data-only element */
	else {
		data = 0;
		path.first = 0;
	}
	if (!(conf = mpt_node_new(path.first+1, path.valid+1))) {
		return 0;
	}
	if (path.first && !mpt_identifier_set(&conf->ident, data, path.first)) {
		mpt_node_destroy(conf);
		return 0;
	}
	/* previous element was section -> insert */
	if (prevop && !(prevop & MPT_PARSEFLAG(SectEnd))) {
		mpt_gnode_insert(old, 0, conf);
	}
	/* no previous or previous element was option -> append */
	else {
		mpt_gnode_add(old, 0, conf);
	}
	
	if (path.valid) {
		if ((data = mpt_path_data(&path))) {
			mpt_node_set(conf, data);
		}
	}
	
	return conf;
}

