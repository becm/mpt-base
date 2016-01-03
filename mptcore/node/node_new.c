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

static void nodeUnref(MPT_INTERFACE(metatype) *meta)
{
	(void) meta;
}
static int nodeAssign(MPT_INTERFACE(metatype) *meta, const MPT_STRUCT(value) *val)
{
	struct _inline_meta *m = (void *) meta;
	if (val) {
		return _mpt_geninfo_value(&m->info, val);
	}
	return 0;
}
static int nodeConv(MPT_INTERFACE(metatype) *meta, int type, void *ptr)
{
	struct _inline_meta *m = (void *) meta;
	void **dest = ptr;
	
	if (type & MPT_ENUM(ValueConsume)) {
		return MPT_ERROR(BadArgument);
	}
	if (!type) {
		static const char types[] = { MPT_ENUM(TypeMeta), MPT_ENUM(TypeNode), 's', 0 };
		if (dest) *dest = (void *) types;
		return 0;
	}
	switch (type & 0xff) {
	  case MPT_ENUM(TypeMeta): ptr = meta; break;
	  case MPT_ENUM(TypeNode): ptr = m->node; break;
	  default: return _mpt_geninfo_conv(&m->info, type, ptr);
	}
	if (dest) *dest = ptr;
	return type & 0xff;
}
static MPT_INTERFACE(metatype) *nodeClone(MPT_INTERFACE(metatype) *meta)
{
	struct _inline_meta *m = (void *) meta;
	return _mpt_geninfo_clone(&m->info);
}

static const MPT_INTERFACE_VPTR(metatype) _meta_control = {
	nodeUnref,
	nodeAssign,
	nodeConv,
	nodeClone
};

extern MPT_STRUCT(node) *mpt_node_new(size_t ilen, size_t dlen)
{
	static const size_t defsize = 16 * sizeof(void*) - sizeof(MPT_STRUCT(node)) + sizeof(MPT_STRUCT(identifier));
	struct _inline_meta *m;
	MPT_STRUCT(node) *node;
	size_t post;
	
	if (!(ilen = mpt_identifier_align(ilen))) {
		return 0;
	}
	ilen = MPT_align(ilen);
	post = ilen + sizeof(*m) + MPT_align(dlen);
	
	if (post > defsize) {
		MPT_INTERFACE(metatype) *meta = 0;
		
		if (dlen && !(meta = mpt_meta_new(dlen))) {
			return 0;
		}
		if (ilen > defsize) {
			ilen = sizeof(node->ident);
		}
		if (!(node = malloc(sizeof(*node) - sizeof(node->ident) + ilen))) {
			if (meta) {
				meta->_vptr->unref(meta);
			}
			return 0;
		}
		node->_meta = meta;
	}
	else {
		uint8_t *base;
		
		if (!(node = malloc(sizeof(*node) - sizeof(node->ident) + post))) {
			return 0;
		}
		base = (void *) (&node->ident);
		m = (void *) (base + ilen);
		
		m->_base._vptr = &_meta_control;
		m->node = node;
		if (_mpt_geninfo_init(&m->info, MPT_align(dlen) + sizeof(m->info)) < 0) {
			free(node);
			return 0;
		}
		node->_meta = &m->_base;
	}
	/* zero relation pointers */
	node->next = node->prev = node->parent = node->children = 0;
	
	mpt_identifier_init(&node->ident, ilen);
	
	return node;
}

