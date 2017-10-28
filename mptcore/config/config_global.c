
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "node.h"
#include "meta.h"
#include "object.h"

#include "config.h"

MPT_STRUCT(configRoot) {
	MPT_INTERFACE(metatype) _mt;
	MPT_INTERFACE(config)   _cfg;
	MPT_STRUCT(path)  base;
};

/* reference interface */
static void configUnref(MPT_INTERFACE(reference) *ref)
{
	MPT_STRUCT(configRoot) *c = (void *) ref;
	free(c);
}
static uintptr_t configRef(MPT_INTERFACE(reference) *ref)
{
	(void) ref;
	return 0;
}
/* special operations for static global instance */
static void configTopUnref(MPT_INTERFACE(reference) *ref)
{
	(void) ref;
}
static uintptr_t configTopRef(MPT_INTERFACE(reference) *ref)
{
	(void) ref;
	return 1;
}
/* metatype interface */
static int configConv(const MPT_INTERFACE(metatype) *mt, int type, void *ptr)
{
	if (!type) {
		static const char fmt[] = { MPT_ENUM(TypeConfig), MPT_ENUM(TypeNode), 0 };
		if (ptr) *((const char **) ptr) = fmt;
		return MPT_ENUM(TypeConfig);
	}
	if (type == MPT_ENUM(TypeConfig)) {
		if (ptr) *((const void **) ptr) = mt + 1;
		return MPT_ENUM(TypeNode);
	}
	if (type == MPT_ENUM(TypeNode)) {
		MPT_STRUCT(configRoot) *c = (void *) mt;
		if (ptr) *((void **) ptr) = mpt_config_node(&c->base);
		return MPT_ENUM(TypeConfig);
	}
	return MPT_ERROR(BadType);
}
static MPT_INTERFACE(metatype) *configClone(const MPT_INTERFACE(metatype) *mt)
{
	MPT_STRUCT(configRoot) *c = (void *) mt;
	return mpt_config_global(&c->base);
}
/* config interface */
static const MPT_INTERFACE(metatype) *configQuery(const MPT_INTERFACE(config) *cfg, const MPT_STRUCT(path) *path)
{
	MPT_STRUCT(configRoot) *c = MPT_baseaddr(configRoot, cfg, _cfg);
	MPT_STRUCT(node) *n;
	MPT_STRUCT(path) p;
	const char *base;
	int len;
	
	if (!(n = mpt_config_node(0))) {
		return 0;
	}
	if (!path) {
		return 0;
	}
	if (c->base.len) {
		p = c->base;
		base = p.base + p.off;
		
		while ((len = mpt_path_next(&p)) >= 0) {
			if (!(n = mpt_node_locate(n, 1, base, len, -1))) {
				return 0;
			}
			if (!p.len && !path->len) {
				return n->_meta;
			}
			if (!(n = n->children)) {
				return 0;
			}
			base = p.base + p.off;
		}
	}
	p = *path;
	base = p.base + p.off;
	
	while ((len = mpt_path_next(&p)) >= 0) {
		if (!(n = mpt_node_locate(n, 1, base, len, -1))) {
			return 0;
		}
		if (!p.len) {
			return n->_meta;
		}
		if (!(n = n->children)) {
			return 0;
		}
		base = p.base + p.off;
	}
	return 0;
}
static int configAssign(MPT_INTERFACE(config) *cfg, const MPT_STRUCT(path) *path, const MPT_STRUCT(value) *val)
{
	MPT_STRUCT(configRoot) *c = MPT_baseaddr(configRoot, cfg, _cfg);
	MPT_INTERFACE(metatype) *mt;
	MPT_STRUCT(node) *n ,*b, *r;
	
	if (!path) {
		return MPT_ERROR(BadArgument);
	}
	if (!(r = b = mpt_config_node(0))) {
		return MPT_ERROR(BadOperation);
	}
	/* get top level node */
	if (c->base.len) {
		MPT_STRUCT(path) p = c->base;
		if (!(b = mpt_node_query(b, &p, 0))) {
			return val ? MPT_ERROR(BadOperation) : 0;
		}
		/* assign top level element */
		if (!path->len) {
			MPT_INTERFACE(metatype) *old;
			if (!val) {
				mt = mpt_metatype_default();
			}
			else if (!(mt = mpt_meta_new(*val))) {
				return MPT_ERROR(BadValue);
			}
			if ((old = b->_meta)) {
				old->_vptr->ref.unref((void *) old);
			}
			b->_meta = mt;
			return mt->_vptr->conv(mt, 0, 0);
		}
		r = b->children;
	}
	/* set subelement */
	if (!(n = mpt_node_assign(&r, path, val))) {
		return val ? MPT_ERROR(BadOperation) : 0;
	}
	if (c->base.len && !b->children) {
		b->children = r;
		r->parent = b;
	}
	mt = n->_meta;
	
	/* return data type identifier */
	return mt ? mt->_vptr->conv(mt, 0, 0) : 0;
}
static int configRemove(MPT_INTERFACE(config) *cfg, const MPT_STRUCT(path) *path)
{
	MPT_STRUCT(configRoot) *c = MPT_baseaddr(configRoot, cfg, _cfg);
	MPT_STRUCT(path) p;
	MPT_STRUCT(node) *b;
	
	if (!(b = mpt_config_node(0))) {
		return MPT_ERROR(BadOperation);
	}
	/* get top level node */
	if (c->base.len) {
		if (!path) {
			return MPT_ERROR(BadArgument);
		}
		p = c->base;
		if (!(b = mpt_node_query(b, &p, 0))) {
			return 0;
		}
	}
	if (!path) {
		mpt_node_clear(b);
		return 0;
	}
	p = *path;
	if (!(b = mpt_node_query(b->children, &p, 0))) {
		return 0;
	}
	mpt_node_unlink(b);
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
		{ configUnref, configRef },
		configConv,
		configClone
	};
	static const MPT_INTERFACE_VPTR(config) configCfg = {
		configQuery,
		configAssign,
		configRemove
	};
	MPT_STRUCT(configRoot) *c;
	MPT_STRUCT(path) p;
	MPT_STRUCT(node) *n;
	
	if (!path) {
		static const MPT_INTERFACE_VPTR(metatype) configMeta = {
			{ configTopUnref, configTopRef },
			configConv,
			configClone,
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
	p = *path;
	p.flags &= ~MPT_PATHFLAG(HasArray);
	
	n = mpt_config_node(0);
	if (!(n = mpt_node_query(n, &p, 0))) {
		MPT_STRUCT(value) val = MPT_VALUE_INIT;
		p = *path;
		if (!(n = mpt_node_query(mpt_config_node(0), &p, &val))) {
			return 0;
		}
		if (p.len && !(n = mpt_node_query(n, &p, 0))) {
			return 0;
		}
	}
	if (!(c = malloc(sizeof(*c) + path->len))) {
		return 0;
	}
	c->_mt._vptr  = &configMeta;
	c->_cfg._vptr = &configCfg;
	memcpy(&c->base, path, sizeof(c->base));
	memcpy(c + 1, path->base + path->off, path->len);
	
	c->base.base  = (char *) (c + 1);
	c->base.off   = 0;
	
	return &c->_mt;
}
