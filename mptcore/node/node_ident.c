/*!
 * node identifier
 */

#include "meta.h"

#include "node.h"

/*!
 * \ingroup mptNode
 * \brief node identifier
 * 
 * Get node identication text.
 * 
 * \param node  node data
 * 
 * \return identifier for node
 */
extern const char *mpt_node_ident(const MPT_STRUCT(node) *node)
{
	int len = mpt_identifier_len(&node->ident);
	return (len <= 0) ? 0 : mpt_identifier_data(&node->ident);
}

/*!
 * \ingroup mptNode
 * \brief node data
 * 
 * Get node data from metatype.
 * Pass zero length pointer to only
 * get printable data.
 * 
 * \param      node  node data
 * \param[out] len   length of node data
 * 
 * \return data for node
 */
extern const char *mpt_node_data(const MPT_STRUCT(node) *node, size_t *len)
{
	return (node && node->_meta) ? mpt_meta_data(node->_meta, len) : 0;
}
