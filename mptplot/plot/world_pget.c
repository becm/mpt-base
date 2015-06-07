/*!
 * get parameter from world structure.
 */

#include <errno.h>
#include <string.h>

#include "plot.h"

/* set/get functions */
static int setCycle(MPT_STRUCT(world) *wld, MPT_INTERFACE(source) *src)
{
	uint16_t cyc;
	int len;
	
	if (!src) return wld->cyc;
	if ((len = src->_vptr->conv(src, 'H', &cyc)) >= 0) {
		wld->cyc = len ? cyc : 0;
	}
	return len;
}

static int setWorld(MPT_STRUCT(world) *wld, MPT_INTERFACE(source) *src)
{
	MPT_STRUCT(world) *from;
	int len;
	
	if ((len = src->_vptr->conv(src, MPT_ENUM(TypeWorld), &from)) >= 0) {
		mpt_world_fini(wld);
		mpt_world_init(wld, from);
		return len;
	}
	if ((len = src->_vptr->conv(src, MPT_ENUM(TypeLineAttr), &wld->attr)) > 0) {
		return len;
	}
	if ((len = src->_vptr->conv(src, MPT_ENUM(TypeColor), &wld->color)) > 0) {
		return len;
	}
	errno = ENOTSUP;
	return -1;
}

/*!
 * \ingroup mptPlot
 * \brief world properties
 * 
 * Get/Set world data elements.
 * 
 * \param world	world data
 * \param pr	property to query
 * \param src	data source to change property
 * 
 * \return consumed/changed value
 */
extern int mpt_world_pget(MPT_STRUCT(world) *world, MPT_STRUCT(property) *pr, MPT_INTERFACE(source) *src)
{
	static const MPT_STRUCT(property) elem[] = {
		{"color",   "world color",   { (char *) mpt_color_pset,   (void *) MPT_offset(world,color) } },
		{"cycles",  "cycle count",   { (char *) setCycle,         (void *) MPT_offset(world,cyc) } },
		{"width",   "line width",    { (char *) mpt_lattr_width,  (void *) MPT_offset(world,attr.width) } }, /* pass world::attr to setter */
		{"style",   "line style",    { (char *) mpt_lattr_style,  (void *) MPT_offset(world,attr.style) } },
		{"symbol",  "symbol type",   { (char *) mpt_lattr_symbol, (void *) MPT_offset(world,attr.symbol) } },
		{"size",    "symbol size",   { (char *) mpt_lattr_size,   (void *) MPT_offset(world,attr.size) } },
		{"alias",   "display name",  { (char *) mpt_text_pset,    (void *) MPT_offset(world,_alias) } },
	};
	static const char format[] = {
		's',
		MPT_ENUM(TypeColor),
		MPT_ENUM(TypeLineAttr),
		'I', 'I',
		0
	};
	MPT_STRUCT(property) self;
	int pos, (*set)();
	
	if (!pr) {
		return src && world ? setWorld(world, src) : MPT_ENUM(TypeWorld);
	}
	self = *pr;
	if (self.name) {
		if (!*self.name) {
			pos = 0;
			if (src && world && (pos = setWorld(world, src)) < 0) {
				return -2;
			}
			pr->name = "world";
			pr->desc = "mpt world data";
			pr->val.fmt = format;
			pr->val.ptr = world;
			
			return pos;
		}
		else if ((pos = mpt_property_match(self.name, 3, elem, MPT_arrsize(elem))) < 0) {
			return -1;
		}
	}
	else if (src) {
		return -2;
	}
	else if ((pos = (intptr_t) pr->desc) < 0 || pos >= (int) MPT_arrsize(elem)) {
		return -1;
	}
	
	set = (int (*)()) elem[pos].val.fmt;
	self.name = elem[pos].name;
	self.desc = elem[pos].desc;
	self.val.ptr = ((uint8_t *) world) + (intptr_t) elem[pos].val.ptr;
	
	if (pos < 1) {
		self.val.fmt = "#";
		if (world && (pos = set(self.val.ptr, src)) < 0) return -2;
	}
	else if (pos < 2) {
		self.val.fmt = "I";
		if (world && (pos = set(world, src)) < 0) return -2;
	}
	else if (pos < 6) {
		self.val.fmt = "C";
		if (world && (pos = set(&world->attr, src)) < 0) return -2;
	}
	else if (!world) {
		self.val.fmt = "s";
	}
	else {
		if ((pos = set(self.val.ptr, src)) < 0) return -2;
		self.val.fmt = 0;
		self.val.ptr = world->_alias;
	}
	*pr = self;
	return pos;
}
