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

struct _inline_meta
{
	MPT_INTERFACE(metatype) _base;
	MPT_STRUCT(node) *node;
};

static void nodeUnref(MPT_INTERFACE(unrefable) *meta)
{
	(void) meta;
}
static int nodeConv(const MPT_INTERFACE(metatype) *meta, int type, void *ptr)
{
	struct _inline_meta *m = (void *) meta;
	void **dest = ptr;
	
	if (!type) {
		static const char types[] = { MPT_ENUM(TypeMeta), MPT_ENUM(TypeNode), 's', 0 };
		if (dest) *dest = (void *) types;
		return 0;
	}
	switch (type) {
	  case MPT_ENUM(TypeMeta): ptr = (void *) meta; break;
	  case MPT_ENUM(TypeNode): ptr = m->node; break;
	  default: return _mpt_geninfo_conv(m + 1, type, ptr);
	}
	if (dest) *dest = ptr;
	return type;
}
static MPT_INTERFACE(metatype) *nodeClone(const MPT_INTERFACE(metatype) *meta)
{
	const struct _inline_meta *m = (void *) meta;
	return _mpt_geninfo_clone(m + 1);
}

static const MPT_INTERFACE_VPTR(metatype) _meta_control = {
	{ nodeUnref },
	nodeConv,
	nodeClone
};

extern MPT_STRUCT(node) *mpt_node_new(size_t ilen, const MPT_STRUCT(value) *val)
{
	static const size_t defsize = 16 * sizeof(void*) - sizeof(MPT_STRUCT(node)) + sizeof(MPT_STRUCT(identifier));
	struct _inline_meta *m;
	MPT_STRUCT(node) *node;
	const char *src;
	size_t post, dlen;
	
	if (!(ilen = mpt_identifier_align(ilen))) {
		return 0;
	}
	src = 0;
	dlen = 0;
	if (val) {
		src = val->ptr;
		if (!val->fmt) {
			if (src) {
				dlen = strlen(val->ptr);
			}
		}
		else if (!(src = mpt_data_tostring((const void **) src, *val->fmt, &dlen))) {
			errno = EINVAL;
			return 0;
		}
	}
	ilen = MPT_align(ilen);
	post = ilen + sizeof(*m) + sizeof(uint64_t) + MPT_align(dlen);
	
	if (!dlen || post > defsize) {
		MPT_INTERFACE(metatype) *meta = 0;
		
		if (val && !(meta = mpt_meta_new(*val))) {
			return 0;
		}
		if (ilen > defsize) {
			ilen = sizeof(node->ident);
		}
		if (!(node = malloc(sizeof(*node) - sizeof(node->ident) + ilen))) {
			if (meta) {
				meta->_vptr->ref.unref((void *) meta);
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
		post -= ilen + sizeof(*m);
		if (_mpt_geninfo_init(m + 1, post) < 0
		    || _mpt_geninfo_set(m + 1, src, dlen) < 0) {
			free(node);
			errno = EINVAL;
			return 0;
		}
		node->_meta = &m->_base;
	}
	/* zero relation pointers */
	node->next = node->prev = node->parent = node->children = 0;
	
	mpt_identifier_init(&node->ident, ilen);
	
	return node;
}

