
#include <sys/uio.h>

#include "node.h"
#include "meta.h"
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
extern MPT_STRUCT(node) *mpt_node_append(MPT_STRUCT(node) *old, const MPT_STRUCT(path) *dest, const MPT_STRUCT(value) *val, int prevop, int currop)
{
	MPT_STRUCT(path) path = *dest;
	MPT_STRUCT(node) *conf;
	MPT_INTERFACE(metatype) *mt;
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
	/* create data for node */
	mt = 0;
	if (val && !(mt = mpt_meta_new(*val))) {
		return 0;
	}
	/* create node with (optional) metadata segment */
	if (!(conf = mpt_node_new(path.first + 1))) {
		if (mt) {
			mt->_vptr->ref.unref((void *) mt);
		}
		return 0;
	}
	conf->_meta = mt;
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
	
	return conf;
}

