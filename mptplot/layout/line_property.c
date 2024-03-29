/*!
 * get properties from line structure.
 */

#include <string.h>
#include <strings.h> /* for strcasecmp() */

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
 * \brief get or register line type
 * 
 * Allocate type for line.
 * 
 * \return ID for type in default namespace
 */
extern int mpt_line_typeid()
{
	static int ptype = 0;
	int type;
	if (!(type = ptype)) {
		static const MPT_STRUCT(type_traits) traits = MPT_TYPETRAIT_INIT(sizeof(def_line));
		if ((type = mpt_type_add(&traits)) > 0) {
			ptype = type;
		}
	}
	return type;
}

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
		int type;
		
		if (!src) {
			return MPT_ERROR(BadOperation);
		}
		if ((type = mpt_line_typeid()) > 0
		 && (len = src->_vptr->convert(src, type, li)) >= 0) {
			if (!len) *li = def_line;
			return 0;
		}
		if ((type = mpt_color_typeid()) > 0
		 && (len = src->_vptr->convert(src, type, &li->color)) >= 0) {
			if (!len) li->color = def_line.color;
			return 0;
		}
		if ((type = mpt_lattr_typeid()) > 0
		 && (len = src->_vptr->convert(src, type, &li->attr)) >= 0) {
			if (!len) li->attr = def_line.attr;
			return 0;
		}
		return MPT_ERROR(BadType);
	}
	/* copy from sibling */
	if (!*name) {
		int type;
		if (!src) {
			*li = def_line;
			return 0;
		}
		if ((type = mpt_color_typeid()) > 0
		 && (len = src->_vptr->convert(src, type, li)) > 0) {
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
	static const struct {
		const char  *name;
		const char  *desc;
		const int    type;
		const size_t off;
	} elem[] = {
		{"color",  "line color",     -1,  MPT_offset(line,color) },
		{"x1",     "line start",     'f', MPT_offset(line,from.x) },
		{"x2",     "line end (x)",   'f', MPT_offset(line,to.x) },
		{"y1",     "line start (y)", 'f', MPT_offset(line,from.y) },
		{"y2",     "line end (y)",   'f', MPT_offset(line,to.y) },
		{"width",  "line width",     'y', MPT_offset(line,attr.width) }, /* pass line::attr to setter */
		{"style",  "line style",     'y', MPT_offset(line,attr.style) },
		{"symbol", "symbol type",    'y', MPT_offset(line,attr.symbol) },
		{"size",   "symbol size",    'y', MPT_offset(line,attr.size) }
	};
	static uint8_t format[] = {
		0,  /* placeholder for dynamic color ID */
		0,  /* placeholder for dynamic line attribute ID */
		'f', 'f', /* line start */
		'f', 'f', /* line end */
		0
	};
	int type;
	int pos;
	
	if (!pr) {
		return mpt_line_typeid();
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
		MPT_value_set(&pr->val, 0, format);
		
		if (!format[0] && (type = mpt_color_typeid()) > 0 && type <= UINT8_MAX) {
			format[0] = type;
		}
		if (!format[1] && (type = mpt_lattr_typeid()) > 0 && type <= UINT8_MAX) {
			format[1] = type;
		}
		
		return li && memcmp(li, &def_line, sizeof(*li)) ? 1 : 0;
	}
	else {
		const char *elem_name[MPT_arrsize(elem)];
		for (pos = 0; pos < (int) MPT_arrsize(elem); pos++) {
			elem_name[pos] = elem[pos].name;
		}
		if ((pos = mpt_property_match(pr->name, -1, elem_name, pos)) < 0) {
			return pos;
		}
	}
	if ((type = elem[pos].type) < 0) {
		if (type == -1) {
			if ((type = mpt_color_typeid()) <= 0) {
				return MPT_ERROR(BadOperation);
			}
		}
		else {
			return MPT_ERROR(BadType);
		}
	}
	pr->name = elem[pos].name;
	pr->desc = elem[pos].desc;
	MPT_value_set(&pr->val, type, ((uint8_t *) li) + elem[pos].off);
	
	if (!li) {
		return 0;
	}
	return mpt_value_compare(&pr->val, ((uint8_t *) &def_line) + elem[pos].off);
}
