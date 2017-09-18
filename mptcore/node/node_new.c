/*!
 * constructor for node.
 * 
 * metatype depends requested size parameters.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>

#include "meta.h"
#include "convert.h"

#include "node.h"


/*!
 * \ingroup mptNode
 * \brief new node
 * 
 * Create new node with optional size for identifier.
 * Base implementation supports text content only
 * 
 * \param post  additional size for data
 * 
 * \return new metatype instance
 */
extern MPT_STRUCT(node) *mpt_node_new(size_t len)
{
	static const size_t preSize = sizeof(MPT_STRUCT(node)) - sizeof(MPT_STRUCT(identifier));
	size_t size;
	MPT_STRUCT(node) *node;
	
	len += preSize;
	size = 8 * sizeof(void *);
	
	/* match node size to identifier */
	if (len > size && len <= 0x100) {
		while (size < len) {
			size *= 2;
		}
	}
	/* use default node allocation size */
	if (!(node = malloc(size))) {
		return 0;
	}
	/* zero relation pointers */
	node->_meta = 0;
	node->next = node->prev = node->parent = node->children = 0;
	
	mpt_identifier_init(&node->ident, size - preSize);
	
	return node;
}

