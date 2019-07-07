
#include <sys/uio.h>

#include "node.h"
#include "meta.h"

#include "config.h"

/*!
 * \ingroup mptConfig
 * \brief set node element
 * 
 * Assign new value to config node tree element.
 * 
 * \param base  configuration list reference
 * \param dest  element destination and data
 * \param val   new element value
 * 
 * \return (new/changed) configuration element
 */
extern MPT_STRUCT(node) *mpt_node_assign(MPT_STRUCT(node) **base, const MPT_STRUCT(path) *dest, const MPT_STRUCT(value) *val)
{
	MPT_STRUCT(path) path = *dest;
	MPT_INTERFACE(metatype) *mt;
	MPT_STRUCT(node) *conf;
	const char *curr;
	int clen;
	
	if ((conf = mpt_node_query(*base, &path))) {
		if (!path.len) {
			if (mpt_meta_set(&conf->_meta, val) < 0) {
				return 0;
			}
			return conf;
		}
		base = &conf->children;
	}
	curr = path.base + path.off;
	
	/* create metatype for value */
	mt = 0;
	if (val && !(mt = mpt_meta_new(*val))) {
		return 0;
	}
	/* require path elements */
	while ((clen = mpt_path_next(&path)) >= 0) {
		MPT_STRUCT(node) *next, *first;
		
		/* create node for current path element */
		if (!(next = mpt_node_new(clen))) {
			break;
		}
		if (!mpt_identifier_set(&next->ident, curr, clen)) {
			mpt_node_destroy(next);
			break;
		}
		/* append/set created path element */
		if (!(first = *base)) {
			next->parent = conf;
			*base = next;
		} else {
			mpt_gnode_add(first, 0, next);
		}
		/* final element */
		if (!path.len) {
			next->_meta = mt;
			return next;
		}
		/* need subelements */
		conf = next;
		base = &next->children;
		curr = path.base + path.off;
		continue;
	}
	if (mt) {
		mt->_vptr->unref(mt);
	}
	return 0;
}
