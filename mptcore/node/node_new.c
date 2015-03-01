/*!
 * constructor for node.
 * 
 * metatype depends requested size parameters.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>

#include "node.h"

struct _inline_meta
{
	MPT_INTERFACE(metatype) _base;
	MPT_STRUCT(node) *node;
	uint64_t info;
};

static MPT_INTERFACE(metatype) *nodeAddref(MPT_INTERFACE(metatype) *meta)
{
	struct _inline_meta *node = (void *) meta;
	
	if (!(meta = mpt_meta_clone(meta))) {
		return 0;
	}
	if ((meta = meta->_vptr->addref(meta))) {
		return node->node->_meta = meta;
	}
	meta->_vptr->unref(meta);
	return 0;
}
static int nodeUnref(MPT_INTERFACE(metatype) *meta)
{ (void) meta; return 0; }

static int nodeProperty(MPT_INTERFACE(metatype) *meta, MPT_STRUCT(property) *prop, MPT_INTERFACE(source) *src)
{
	struct _inline_meta *m = (void *) meta;
	if (prop || src) {
		return _mpt_geninfo_property(&m->info, prop, src);
	}
	return 0;
}

static void *nodeCast(MPT_INTERFACE(metatype) *meta, int type)
{
	struct _inline_meta *m = (void *) meta;
	switch (type) {
	  case MPT_ENUM(TypeMeta): return meta;
	  case MPT_ENUM(TypeNode): return m->node;
	  case 's': return _mpt_geninfo_property(&m->info, 0, 0) > 0 ? (&m->info) + 1 : 0;
	  default: return 0;
	}
}

static const MPT_INTERFACE_VPTR(metatype) _meta_control = {
	nodeUnref,
	nodeAddref,
	nodeProperty,
	nodeCast
};

extern MPT_STRUCT(node) *mpt_node_new(size_t dlen, const char *ident, int idlen)
{
	static const size_t defsize = 16 * sizeof(void*) - sizeof(MPT_STRUCT(node));
	struct _inline_meta *m;
	MPT_STRUCT(node) *node;
	size_t post, ilen;
	
	if (!(ilen = mpt_identifier_align(idlen > 0 ? idlen + 1 : -idlen))) {
		return 0;
	}
	ilen = MPT_align(ilen);
	post = MPT_align(dlen) + ilen + sizeof(*m);
	
	if (!dlen || post > defsize) {
		MPT_INTERFACE(metatype) *meta = 0;
		
		if (dlen && !(meta = mpt_meta_new(dlen))) {
			return 0;
		}
		if (ilen > defsize) {
			ilen = 0;
		}
		if (!(node = malloc(sizeof(*node) + ilen))) {
			if (meta) {
				meta->_vptr->unref(meta);
			}
			return 0;
		}
		node->_meta = meta;
	}
	else {
		uint8_t *base;
		
		if (!(node = malloc(sizeof(*node) + post))) {
			return 0;
		}
		base = (void *) (node + 1);
		m = (void *) (base + ilen);
		
		m->_base._vptr = &_meta_control;
		m->node = node;
		if (_mpt_geninfo_init(&m->info, MPT_align(dlen) + sizeof(m->info), 1) < 0) {
			free(node);
			return 0;
		}
		node->_meta = &m->_base;
	}
	/* zero relation pointers */
	node->next = node->prev = node->parent = node->children = 0;
	
	mpt_identifier_init(&node->ident, ilen);
	mpt_identifier_set(&node->ident, ident, idlen);
	
	return node;
}

