
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
	MPT_INTERFACE(metatype) *me;
	MPT_STRUCT(path)  base;
};
static MPT_STRUCT(node) *nodeGlobal = 0;

/* reference interface */
static void configUnref(MPT_INTERFACE(reference) *ref)
{
	MPT_STRUCT(configRoot) *c = (void *) ref;
	MPT_INTERFACE(metatype) *mt;
	mpt_path_fini(&c->base);
	if ((mt = c->me)) {
		mt->_vptr->ref.unref((void *) mt);
	}
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
	const MPT_STRUCT(configRoot) *c = (void *) mt;
	if (!type) {
		static const char fmt[] = { MPT_ENUM(TypeConfig), MPT_ENUM(TypeNode), 0 };
		if (ptr) *((const char **) ptr) = fmt;
		return MPT_ENUM(TypeConfig);
	}
	if (type == MPT_ENUM(TypeConfig)) {
		if (ptr) *((const void **) ptr) = &c->_cfg;
		return MPT_ENUM(TypeNode);
	}
	if (type == MPT_ENUM(TypeNode)) {
		if (!nodeGlobal || !c->base.len) {
			return MPT_ERROR(BadValue);
		}
		if (ptr) {
			MPT_STRUCT(path) p = c->base;
			*((void **) ptr) = mpt_node_query(nodeGlobal, &p, 0);
		}
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
	MPT_INTERFACE(metatype) *mt;
	MPT_STRUCT(node) *n;
	
	if (!path) {
		return &c->_mt;
	}
	if (!(n = nodeGlobal)) {
		return 0;
	}
	if (!(mt = c->me)) {
		mt = mpt_metatype_default();
	}
	if (c->base.len) {
		MPT_STRUCT(path) p = c->base;
		if (!(n = mpt_node_query(n, &p, 0))) {
			return 0;
		}
		if (!(n = n->children)) {
			return 0;
		}
		if (n->_meta) {
			mt = n->_meta;
		}
	}
	if (path->len) {
		MPT_STRUCT(path) p = *path;
		if (!(n = mpt_node_query(n, &p, 0))) {
			return 0;
		}
		if (n->_meta) {
			mt = n->_meta;
		}
	}
	return mt;
}
static int configAssign(MPT_INTERFACE(config) *cfg, const MPT_STRUCT(path) *path, const MPT_STRUCT(value) *val)
{
	MPT_STRUCT(configRoot) *c = MPT_baseaddr(configRoot, cfg, _cfg);
	MPT_INTERFACE(metatype) *mt;
	MPT_STRUCT(node) **b, *n;
	
	if (!path) {
		return MPT_ERROR(BadArgument);
	}
	n = nodeGlobal;
	b = &nodeGlobal;
	
	/* get/create config base node */
	if (c->base.len) {
		MPT_STRUCT(path) p = c->base;
		if (!(n = mpt_node_assign(b, &p, 0))) {
			return val ? MPT_ERROR(BadOperation) : 0;
		}
		b = &n->children;
	}
	/* top level element */
	if (!path->len) {
		MPT_INTERFACE(metatype) *old;
		if (!val) {
			mt = mpt_metatype_default();
		}
		else if (!(mt = mpt_meta_new(*val))) {
			return MPT_ERROR(BadValue);
		}
		if (c->base.len) {
			old = n->_meta;
			n->_meta = mt;
		} else {
			old = c->me;
			c->me = mt;
		}
		if (old) {
			old->_vptr->ref.unref((void *) old);
		}
		return mt->_vptr->conv(mt, 0, 0);
	}
	/* set subelement */
	if (!(n = mpt_node_assign(b, path, val))) {
		return val ? MPT_ERROR(BadOperation) : 0;
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
	
	if (!(b = nodeGlobal)) {
		return MPT_ERROR(BadOperation);
	}
	/* get top level node */
	if (c->base.len) {
		p = c->base;
		if (!(b = mpt_node_query(b, &p, 0))) {
			return 0;
		}
		if (!path) {
			mpt_node_clear(b);
			return 0;
		}
		b = b->children;
	}
	/* clear global config */
	else if (!path) {
		MPT_STRUCT(node) tmp = MPT_NODE_INIT;
		tmp.children = b;
		mpt_node_clear(&tmp);
		nodeGlobal = 0;
		return 0;
	}
	if (!path->len) {
		MPT_INTERFACE(metatype) *mt;
		if (!(mt = c->me)) {
			return MPT_ERROR(MissingData);
		} else {
			c->me = 0;
			mt->_vptr->ref.unref((void *) mt);
			return 0;
		}
	}
	p = *path;
	if (!(b = mpt_node_query(b, &p, 0))) {
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
			0, MPT_PATH_INIT
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
	
	if (!(n = mpt_node_query(nodeGlobal, &p, 0))) {
		if (!(n = mpt_node_assign(&nodeGlobal, path, 0))) {
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
