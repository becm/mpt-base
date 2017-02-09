/*!
 * get properties from line structure.
 */

#include <string.h>
#include <strings.h> /* for strcasecmp() */

#include "meta.h"

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
static int setPosition(float *val, MPT_INTERFACE(metatype) *src)
{
	int len;
	if (!src) {
		*val = 0;
		return 0;
	}
	if (!(len = src->_vptr->conv(src, 'f' | MPT_ENUM(ValueConsume), val))) {
		*val = 0.0;
	}
	return len < 0 ? len : 1;;
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
extern int mpt_line_set(MPT_STRUCT(line) *li, const char *name, MPT_INTERFACE(metatype) *src)
{
	int len;
	
	/* auto-select matching property */
	if (!name) {
		const MPT_STRUCT(line) *from;
		
		if (!src) {
			return MPT_ERROR(BadOperation);
		}
		if ((len = src->_vptr->conv(src, MPT_ENUM(TypeLine) | MPT_ENUM(ValueConsume), &from)) >= 0) {
			*li = from ? *from : def_line;
			return len ? 1 : 0;
		}
		if ((len = src->_vptr->conv(src, MPT_ENUM(TypeColor) | MPT_ENUM(ValueConsume), &li->color)) >= 0) {
			return len ? 1 : 0;
		}
		if ((len = src->_vptr->conv(src, MPT_ENUM(TypeLineAttr) | MPT_ENUM(ValueConsume), &li->attr)) >= 0) {
			return len ? 1 : 0;
		}
		return MPT_ERROR(BadType);
	}
	/* copy from sibling */
	if (!*name) {
		const MPT_STRUCT(line) *from;
		
		if (!src) {
			*li = def_line;
			return 0;
		}
		if ((len = src->_vptr->conv(src, MPT_ENUM(TypeText) | MPT_ENUM(ValueConsume), &from)) >= 0) {
			*li = from ? *from : def_line;
			return len ? 1 : 0;
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
	static const char cfmt[2] = { MPT_ENUM(TypeColor) };
	static const MPT_STRUCT(property) elem[] = {
		{"color",  "line color",     { cfmt, (void *) MPT_offset(line,color)} },
		{"x1",     "line start",     { "f",  (void *) MPT_offset(line,from.x)} },
		{"x2",     "line end (x)",   { "f",  (void *) MPT_offset(line,to.x)} },
		{"y1",     "line start (y)", { "f",  (void *) MPT_offset(line,from.y)} },
		{"y2",     "line end (y)",   { "f",  (void *) MPT_offset(line,to.y)} },
		{"width",  "line width",     { "y",  (void *) MPT_offset(line,attr.width)} }, /* pass line::attr to setter */
		{"style",  "line style",     { "y",  (void *) MPT_offset(line,attr.style)} },
		{"symbol", "symbol type",    { "y",  (void *) MPT_offset(line,attr.symbol)} },
		{"size",   "symbol size",    { "y",  (void *) MPT_offset(line,attr.size)} }
	};
	static const char format[] = {
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
		pr->val.fmt = format;
		pr->val.ptr = li;
		
		return MPT_ENUM(TypeLine);
	}
	else if ((pos = mpt_property_match(pr->name, -1, elem, MPT_arrsize(elem))) < 0) {
		return pos;
	}
	pr->name = elem[pos].name;
	pr->desc = elem[pos].desc;
	pr->val.fmt = elem[pos].val.fmt;
	pr->val.ptr = ((uint8_t *) li) + (intptr_t) elem[pos].val.ptr;
	
	if (!li) {
		return 0;
	}
	return mpt_value_compare(&pr->val, ((uint8_t *) &def_line) + (intptr_t) elem[pos].val.ptr);
}
