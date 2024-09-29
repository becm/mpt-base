
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "collection.h"
#include "node.h"
#include "meta.h"
#include "object.h"
#include "types.h"

#include "config.h"

MPT_STRUCT(configRoot) {
	MPT_INTERFACE(metatype) _mt;
	MPT_INTERFACE(config)   _cfg;
	MPT_STRUCT(path) base;
};
static MPT_STRUCT(node) *nodeGlobal = 0;

static void clear_global(void)
{
	MPT_STRUCT(node) tmp = MPT_NODE_INIT;
	if (!(tmp.children = nodeGlobal)) {
		return;
	}
	mpt_node_clear(&tmp);
	nodeGlobal = 0;
}
static MPT_STRUCT(node) *make_global(const MPT_STRUCT(path) *dest)
{
	static int endreg = 0;
	
	MPT_STRUCT(path) p = *dest;
	MPT_STRUCT(node) *n;
	
	if (!endreg) {
		atexit(clear_global);
		endreg = 1;
	}
	
	if (!(n = mpt_node_query(nodeGlobal, &p))) {
		return mpt_node_assign(&nodeGlobal, dest, 0);
	}
	if (p.len) {
		MPT_STRUCT(node) *t = n->children ? 0 : n;
		if (!(n = mpt_node_assign(&n->children, &p, 0))) {
			return 0;
		}
		if (t) {
			t->children->parent = t;
		}
	}
	return n;
}

/* reference interface */
static void configUnref(MPT_INTERFACE(metatype) *mt)
{
	MPT_STRUCT(configRoot) *c = (void *) mt;
	mpt_path_fini(&c->base);
	free(c);
}
static uintptr_t configRef(MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* special operations for static global instance */
static void configTopUnref(MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
}
static uintptr_t configTopRef(MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 1;
}
/* metatype interface */
static int configConv(MPT_INTERFACE(convertable) *val, uintptr_t type, void *ptr)
{
	const MPT_STRUCT(configRoot) *c = (void *) val;
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeConfigPtr), MPT_ENUM(TypeNodePtr), 0 };
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
			return 0;
		}
		return MPT_ENUM(TypeConfigPtr);
	}
	if (type == MPT_ENUM(TypeConfigPtr)) {
		if (ptr) *((const void **) ptr) = &c->_cfg;
		return MPT_ENUM(TypeNodePtr);
	}
	if (type == MPT_ENUM(TypeNodePtr)) {
		if (!c->base.len) {
			return MPT_ERROR(BadValue);
		}
		if (ptr) {
			*((void **) ptr) = make_global(&c->base);
		}
		return MPT_ENUM(TypeConfigPtr);
	}
	return MPT_ERROR(BadType);
}
static MPT_INTERFACE(metatype) *configClone(const MPT_INTERFACE(metatype) *mt)
{
	MPT_STRUCT(configRoot) *c = (void *) mt;
	
	return mpt_config_global(&c->base);
}
/* config interface */
struct _node_collection {
	MPT_INTERFACE(collection) col;
	MPT_STRUCT(node) *n;
};
static int collectionEach(const MPT_INTERFACE(collection) *c, MPT_TYPE(item_handler) handler, void *ctx)
{
	struct _node_collection *col = (void *) c;
	MPT_STRUCT(node) *n;
	const MPT_INTERFACE_VPTR(collection) cvptr = { collectionEach };
	
	if (!(n = col->n)) {
		return 0;
	}
	while (n) {
		struct _node_collection col = { { &cvptr }, n->children };
		int ret = handler(ctx, &n->ident, (MPT_INTERFACE(convertable) *) n->_meta, &col.col);
		if (ret < 0) {
			return ret;
		}
		n = n->next;
	}
	return 0;
}
static int configQuery(const MPT_INTERFACE(config) *cfg, const MPT_STRUCT(path) *path, MPT_TYPE(config_handler) fcn, void *ctx)
{
	static const MPT_INTERFACE_VPTR(collection) cvptr = { collectionEach };
	MPT_STRUCT(configRoot) *c = MPT_baseaddr(configRoot, cfg, _cfg);
	MPT_STRUCT(node) *n = nodeGlobal;
	MPT_INTERFACE(metatype) *mt;
	
	if (!path) {
		return fcn ? fcn(ctx, (MPT_INTERFACE(convertable) *) &c->_mt, 0) : 0;
	}
	mt = 0;
	if (c->base.len) {
		MPT_STRUCT(path) p = c->base;
		if (!(n = mpt_node_query(n, &p))
		 || p.len) {
			return MPT_ERROR(MissingData);
		}
		mt = n->_meta;
		n = n->children;
	}
	if (path->len) {
		MPT_STRUCT(path) p = *path;
		if (!(n = mpt_node_query(n, &p))
		 || p.len) {
			return MPT_ERROR(MissingData);
		}
		mt = n->_meta;
		n = n->children;
	}
	/* existence is verified */
	if (!fcn) {
		return 0;
	}
	if (n) {
		struct _node_collection col = { { &cvptr }, n };
		return fcn(ctx, (MPT_INTERFACE(convertable) *) mt, &col.col);
	}
	return fcn(ctx, (MPT_INTERFACE(convertable) *) mt, 0);
}
static int configAssign(MPT_INTERFACE(config) *cfg, const MPT_STRUCT(path) *path, const MPT_STRUCT(value) *val)
{
	MPT_STRUCT(configRoot) *c = MPT_baseaddr(configRoot, cfg, _cfg);
	MPT_INTERFACE(metatype) *mt;
	MPT_STRUCT(node) **b, *n, *t;
	
	if (!path) {
		return MPT_ERROR(BadArgument);
	}
	n = nodeGlobal;
	b = &nodeGlobal;
	t = 0;
	
	/* get/create config base node */
	if (c->base.len) {
		if (!(n = make_global(&c->base))) {
			return MPT_ERROR(BadOperation);
		}
		t = n->children ? 0 : n;
		b = &n->children;
	}
	/* top level element */
	if (!path->len) {
		if (!c->base.len) {
			return MPT_ERROR(BadValue);
		}
		return mpt_meta_set(&n->_meta, val);
	}
	/* set subelement */
	if (!(n = mpt_node_assign(b, path, val))) {
		return val ? MPT_ERROR(BadOperation) : 0;
	}
	if (t) {
		t->children->parent = t;
	}
	mt = n->_meta;
	
	/* return data type identifier */
	return mt ? MPT_metatype_convert(mt, 0, 0) : 0;
}
static int configRemove(MPT_INTERFACE(config) *cfg, const MPT_STRUCT(path) *path)
{
	MPT_STRUCT(configRoot) *c = MPT_baseaddr(configRoot, cfg, _cfg);
	MPT_STRUCT(path) p;
	MPT_STRUCT(node) *b;
	
	if (!(b = nodeGlobal)) {
		return MPT_ERROR(BadOperation);
	}
	/* get top level node */
	if (c->base.len) {
		p = c->base;
		if (!(b = mpt_node_query(b, &p))
		    || p.len) {
			return 0;
		}
		if (!path) {
			MPT_INTERFACE(metatype) *mt;
			if ((mt = b->_meta)) {
				mt->_vptr->unref(mt);
				b->_meta = 0;
			}
			return 0;
		}
		if (!path->len) {
			mpt_node_clear(b);
			return 0;
		}
		b = b->children;
	}
	/* no generic top element */
	else if (!path) {
		return MPT_ERROR(BadOperation);
	}
	/* clear global config */
	else if (!path->len) {
		clear_global();
		return 0;
	}
	p = *path;
	if (!(b = mpt_node_query(b, &p))
	    || p.len) {
		return 0;
	}
	if (b == nodeGlobal) {
		nodeGlobal = b->next;
		b->next = 0;
	} else {
		mpt_node_unlink(b);
	}
	mpt_node_destroy(b);
	return 1;
}

/*!
 * \ingroup mptConfig
 * \brief get configuration element
 * 
 * find element in global configuration tree/list.
 * 
 * \param path  element path
 * 
 * \return config node if exists
 */
extern MPT_INTERFACE(metatype) *mpt_config_global(const MPT_STRUCT(path) *path)
{
	static const MPT_INTERFACE_VPTR(metatype) configMeta = {
		{ configConv },
		configUnref,
		configRef,
		configClone
	};
	static const MPT_INTERFACE_VPTR(config) configCfg = {
		configQuery,
		configAssign,
		configRemove
	};
	MPT_STRUCT(configRoot) *c;
	
	if (!path) {
		static const MPT_INTERFACE_VPTR(metatype) configMeta = {
			{ configConv },
			configTopUnref,
			configTopRef,
			configClone
		};
		static MPT_STRUCT(configRoot) global = {
			{ &configMeta },
			{ &configCfg },
			MPT_PATH_INIT
		};
		return &global._mt;
	}
	/* missing path information */
	if (!path->len) {
		errno = EINVAL;
		return 0;
	}
	
	/* new global config target */
	if (!(c = malloc(sizeof(*c) + path->len))) {
		return 0;
	}
	
	c->_mt._vptr  = &configMeta;
	c->_cfg._vptr = &configCfg;
	
	c->base = *path;
	c->base.flags &= ~MPT_PATHFLAG(HasArray);
	c->base.base = memcpy(c + 1, path->base + path->off, path->len);
	c->base.off = 0;
	
	return &c->_mt;
}
