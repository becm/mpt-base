/*!
 * get properties from line structure.
 */

#include <string.h>
#include <strings.h> /* for strcasecmp() */

#include "meta.h"
#include "types.h"
#include "object.h"

#include "layout.h"

static const MPT_STRUCT(line) def_line = {
	MPT_COLOR_INIT,    /* color */
	MPT_LINEATTR_INIT, /* attr */
	{ 0, 0 },          /* from */
	{ 0, 0 }           /* to */
};

/*!
 * \ingroup mptPlot
 * \brief initialize line structure
 * 
 * Set default values for line members.
 * 
 * \param line  uninitialized line data
 */
extern void mpt_line_init(MPT_STRUCT(line) *line)
{
	*line = def_line;
}
/* set/get functions */
static int setPosition(float *val, MPT_INTERFACE(convertable) *src)
{
	double tmp;
	int len;
	if (!src) {
		*val = 0;
		return 0;
	}
	if ((len = src->_vptr->convert(src, 'f', val)) >= 0) {
		if (!len) *val = 0.0f;
		return 0;
	}
	if ((len = src->_vptr->convert(src, 'd', &tmp)) >= 0) {
		if (!len) {
			*val = 0.0f;
		} else {
			*val = tmp;
		}
		return 0;
	}
	return MPT_ERROR(BadType);
}
/*!
 * \ingroup mptPlot
 * \brief set line properties
 * 
 * Change or reset line structure properties.
 * 
 * \param li   line data
 * \param name property name
 * \param src  value source
 */
extern int mpt_line_set(MPT_STRUCT(line) *li, const char *name, MPT_INTERFACE(convertable) *src)
{
	int len;
	
	/* auto-select matching property */
	if (!name) {
		const MPT_STRUCT(line) *from;
		
		if (!src) {
			return MPT_ERROR(BadOperation);
		}
		if ((len = src->_vptr->convert(src, MPT_ENUM(TypeLine), &from)) >= 0) {
			*li = from ? *from : def_line;
			return 0;
		}
		if ((len = src->_vptr->convert(src, MPT_ENUM(TypeColor), &li->color)) >= 0) {
			if (!len) li->color = def_line.color;
			return 0;
		}
		if ((len = src->_vptr->convert(src, MPT_ENUM(TypeLineAttr), &li->attr)) >= 0) {
			if (!len) li->attr = def_line.attr;
			return 0;
		}
		return MPT_ERROR(BadType);
	}
	/* copy from sibling */
	if (!*name) {
		if (!src || !(len = src->_vptr->convert(src, MPT_ENUM(TypeLine), li))) {
			*li = def_line;
			return 0;
		}
		if (len) {
			return 0;
		}
		return MPT_ERROR(BadType);
	}
	if (!strcmp(name, "x1")) {
		return setPosition(&li->from.x, src);
	}
	if (!strcmp(name, "x2")) {
		return setPosition(&li->to.x, src);
	}
	if (!strcmp(name, "y1")) {
		return setPosition(&li->from.y, src);
	}
	if (!strcmp(name, "y2")) {
		return setPosition(&li->to.y, src);
	}
	if (!strcasecmp(name, "color")) {
		return mpt_color_pset(&li->color, src);
	}
	if (!strcasecmp(name, "width")) {
		return mpt_lattr_width(&li->attr, src);
	}
	if (!strcasecmp(name, "style")) {
		return mpt_lattr_style(&li->attr, src);
	}
	if (!strcasecmp(name, "symbol")) {
		return mpt_lattr_symbol(&li->attr, src);
	}
	if (!strcasecmp(name, "size")) {
		return mpt_lattr_size(&li->attr, src);
	}
	return MPT_ERROR(BadArgument);
}
/*!
 * \ingroup mptPlot
 * \brief get line properties
 * 
 * Get line property parameters.
 * 
 * \param line  line data
 * \param pr    property to query
 * 
 * \return consumed/changed value
 */
extern int mpt_line_get(const MPT_STRUCT(line) *li, MPT_STRUCT(property) *pr)
{
	static const uint8_t cfmt = MPT_ENUM(TypeColor);
	static const MPT_STRUCT(property) elem[] = {
		{"color",  "line color",     MPT_VALUE_INIT(cfmt,  (void *) MPT_offset(line,color)) },
		{"x1",     "line start",     MPT_VALUE_INIT('f',   (void *) MPT_offset(line,from.x)) },
		{"x2",     "line end (x)",   MPT_VALUE_INIT('f',   (void *) MPT_offset(line,to.x)) },
		{"y1",     "line start (y)", MPT_VALUE_INIT('f',   (void *) MPT_offset(line,from.y)) },
		{"y2",     "line end (y)",   MPT_VALUE_INIT('f',   (void *) MPT_offset(line,to.y)) },
		{"width",  "line width",     MPT_VALUE_INIT('y',   (void *) MPT_offset(line,attr.width)) }, /* pass line::attr to setter */
		{"style",  "line style",     MPT_VALUE_INIT('y',   (void *) MPT_offset(line,attr.style)) },
		{"symbol", "symbol type",    MPT_VALUE_INIT('y',   (void *) MPT_offset(line,attr.symbol)) },
		{"size",   "symbol size",    MPT_VALUE_INIT('y',   (void *) MPT_offset(line,attr.size)) }
	};
	static const uint8_t format[] = {
		MPT_ENUM(TypeColor),
		MPT_ENUM(TypeLineAttr),
		'f', 'f', /* line start */
		'f', 'f', /* line end */
		0
	};
	int pos;
	
	if (!pr) {
		return MPT_ENUM(TypeLine);
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
		pr->name = "line";
		pr->desc = "mpt line data";
		pr->val.type = 0;
		pr->val.ptr  = format;
		
		return li && memcmp(li, &def_line, sizeof(*li)) ? 1 : 0;
	}
	else if ((pos = mpt_property_match(pr->name, -1, elem, MPT_arrsize(elem))) < 0) {
		return pos;
	}
	pr->name = elem[pos].name;
	pr->desc = elem[pos].desc;
	pr->val.type = elem[pos].val.type;
	pr->val.ptr  = ((uint8_t *) li) + (intptr_t) elem[pos].val.ptr;
	
	if (!li) {
		return 0;
	}
	return mpt_value_compare(&pr->val, ((uint8_t *) &def_line) + (intptr_t) elem[pos].val.ptr);
}
