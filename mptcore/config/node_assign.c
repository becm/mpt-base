
#include <sys/uio.h>

#include "node.h"

#include "config.h"

/*!
 * \ingroup mptConfig
 * \brief insert path element
 * 
 * insert path element into tree.
 * 
 * \param base  configuration list reference
 * \param dest  element destination and data
 * 
 * \return (new/changed) configuration list
 */
extern MPT_STRUCT(node) *mpt_node_assign(MPT_STRUCT(node) **base, const MPT_STRUCT(path) *dest)
{
	static const char vecChar[2] = { MPT_value_toVector('c') };
	struct iovec vec;
	MPT_STRUCT(value) val;
	MPT_STRUCT(path) path = *dest;
	MPT_STRUCT(node) *conf;
	
	vec.iov_len = path.valid;
	vec.iov_base = (char *) mpt_path_data(dest);
	val.ptr = &vec;
	val.fmt = vecChar;
	
	if (!(conf = mpt_node_query(*base, &path, &val))) {
		return 0;
	}
	if (!*base) *base = conf;
	
	if (path.len && !(conf = mpt_node_query(conf->children, &path, 0))) {
		return 0;
	}
	return conf;
}

