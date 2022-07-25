/*!
 * get parameter from world structure.
 */

#define _POSIX_C_SOURCE 200809L /* need for strdup() */

#include <string.h>
#include <strings.h> /* for strcasecmp() */
#include <stdlib.h>

#include "meta.h"
#include "types.h"
#include "object.h"

#include "layout.h"

static const MPT_STRUCT(world) def_world = {
	0,                  /* _alias */
	MPT_COLOR_INIT,     /* color */
	MPT_LINEATTR_INIT,  /* attr */
	0                   /* cyc */
};
/*!
 * \ingroup mptPlot
 * \brief initialize world structure
 * 
 * Set default values for world members.
 * 
 * \param wld   world data
 * \param from  copy data template
 */
extern void mpt_world_init(MPT_STRUCT(world) *wld, const MPT_STRUCT(world) *from)
{
	if (from) {
		*wld = *from;
		
		if (wld->_alias) wld->_alias = strdup(wld->_alias);
		
		return;
	}
	*wld = def_world;
}

/*!
 * \ingroup mptPlot
 * \brief finalize world structure
 * 
 * Clear allocated resources.
 * 
 * \param wld  world data
 */
extern void mpt_world_fini(MPT_STRUCT(world) *wld)
{
	free(wld->_alias);
	*wld = def_world;
}

/*!
 * \ingroup mptPlot
 * \brief set world properties
 * 
 * Change or reset world structure properties.
 * 
 * \param wld  world data
 * \param name property name
 * \param src  value source
 */
extern int mpt_world_set(MPT_STRUCT(world) *wld, const char *name, MPT_INTERFACE(convertable) *src)
{
	int len;
	
	if (!name) {
		const MPT_STRUCT(world) *from;
		
		if (!src) {
			return MPT_ERROR(BadOperation);
		}
		if ((len = src->_vptr->convert(src, MPT_ENUM(TypeWorldPtr), &from)) >= 0) {
			mpt_world_fini(wld);
			mpt_world_init(wld, len ? from : 0);
			return 0;
		}
		if ((len = mpt_string_pset(&wld->_alias, src)) >= 0) {
			return len;
		}
		if ((len = src->_vptr->convert(src, MPT_ENUM(TypeColor), &wld->color)) >= 0) {
			if (!len) wld->color = def_world.color;
			return 0;
		}
		if ((len = src->_vptr->convert(src, MPT_ENUM(TypeLineAttr), &wld->attr)) >= 0) {
			if (!len) wld->attr = def_world.attr;
			return 0;
		}
		return MPT_ERROR(BadType);
	}
	/* copy from sibling */
	if (!*name) {
		const MPT_STRUCT(world) *from;
		
		if (!src) {
			mpt_world_fini(wld);
			return 0;
		}
		if ((len = src->_vptr->convert(src, MPT_ENUM(TypeWorldPtr), &from)) >= 0) {
			mpt_world_fini(wld);
			mpt_world_init(wld, len ? from : 0);
			return 0;
		}
		return MPT_ERROR(BadType);
	}
	if (!strcasecmp(name, "cyc") || !strcasecmp(name, "cycles")) {
		if (!src || !(len = src->_vptr->convert(src, 'u', &wld->cyc))) {
			wld->cyc = def_world.cyc;
			return 0;
		}
		return len < 0 ? len : 0;
	}
	if (!strcasecmp(name, "color") || !strcasecmp(name, "colour")) {
		if (!src) {
			wld->cyc = def_world.cyc;
			return 0;
		}
		return mpt_color_pset(&wld->color, src);
	}
	if (!strcasecmp(name, "alias")) {
		if (!src) {
			mpt_string_set(&wld->_alias, 0, 0);
			return 0;
		}
		return mpt_string_pset(&wld->_alias, src);
	}
	if (!strcasecmp(name, "width")) {
		if (!src) {
			wld->attr.width = def_world.attr.width;
			return 0;
		}
		return mpt_lattr_width(&wld->attr, src);
	}
	if (!strcasecmp(name, "style")) {
		if (!src) {
			wld->attr.style = def_world.attr.style;
			return 0;
		}
		return mpt_lattr_style(&wld->attr, src);
	}
	if (!strcasecmp(name, "sym") || !strcasecmp(name, "symbol")) {
		if (!src) {
			wld->attr.symbol = def_world.attr.symbol;
			return 0;
		}
		return mpt_lattr_symbol(&wld->attr, src);
	}
	if (!strcasecmp(name, "size")) {
		if (!src) {
			wld->attr.size = def_world.attr.size;
			return 0;
		}
		return mpt_lattr_size(&wld->attr, src);
	}
	return MPT_ERROR(BadArgument);
}

/*!
 * \ingroup mptPlot
 * \brief get world properties
 * 
 * Get world property parameters.
 * 
 * \param world world data
 * \param pr    property to query
 * 
 * \return consumed/changed value
 */
extern int mpt_world_get(const MPT_STRUCT(world) *wld, MPT_STRUCT(property) *pr)
{
	static const struct {
		const char  *name;
		const char  *desc;
		const int    type;
		const size_t off;
	} elem[] = {
		{"color",   "world color",   MPT_ENUM(TypeColor), MPT_offset(world,color) },
		{"cycles",  "cycle count",   'u', MPT_offset(world,cyc) },
		{"width",   "line width",    'y', MPT_offset(world,attr.width) },
		{"style",   "line style",    'y', MPT_offset(world,attr.style) },
		{"symbol",  "symbol type",   'y', MPT_offset(world,attr.symbol) },
		{"size",    "symbol size",   'y', MPT_offset(world,attr.size) },
		{"alias",   "display name",  's', MPT_offset(world,_alias) },
	};
	static const uint8_t format[] = {
		's',
		MPT_ENUM(TypeColor),
		MPT_ENUM(TypeLineAttr),
		'u',
		0
	};
	int pos;
	
	if (!pr) {
		return MPT_ENUM(TypeWorldPtr);
	}
	/* property by position */
	if (!pr->name) {
		pos = (intptr_t) pr->desc;
		if (pos < 0 || pos >= (int) MPT_arrsize(elem)) {
			return MPT_ERROR(BadArgument);
		}
	}
	/* get total type */
	else if (!*pr->name) {
		pr->name = "world";
		pr->desc = "mpt world data";
		pr->val.type = 0;
		pr->val.ptr  = format;
		
		return wld && memcmp(wld, &def_world, sizeof(*wld)) ? 1 : 0;
	}
	/* find property by name */
	else {
		const char *elem_name[MPT_arrsize(elem)];
		for (pos = 0; pos < (int) MPT_arrsize(elem); pos++) {
			elem_name[pos] = elem[pos].name;
		}
		if ((pos = mpt_property_match(pr->name, 3, elem_name, pos)) < 0) {
			return pos;
		}
	}
	pr->name = elem[pos].name;
	pr->desc = elem[pos].desc;
	MPT_value_set(&pr->val, elem[pos].type, ((uint8_t *) wld) + elem[pos].off);
	
	if (!wld) {
		return 0;
	}
	return mpt_value_compare(&pr->val, ((uint8_t *) &def_world) + elem[pos].off);
}
