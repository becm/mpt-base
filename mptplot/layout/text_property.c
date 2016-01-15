/*!
 * get properties from text structure.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "layout.h"

/* default text values */
static const MPT_STRUCT(text) def_text = {
        0,              /* data pointer */
        0,              /* text family */
        MPT_COLOR_INIT, /* text color (rgba) */
        10,             /* size */
        'n', 'n',       /* style, weight */
        '5',            /* align */
        { 0.5, 0.5 },   /* pos */
        0.0             /* angle */
};

/*!
 * \ingroup mptPlot
 * \brief finalize text
 * 
 * Clear resources of text data.
 * 
 * \param tx  text data
 */
extern void mpt_text_fini(MPT_STRUCT(text) *tx)
{
	free(tx->_value);
	free(tx->_font);
	*tx = def_text;
}

/*!
 * \ingroup mptPlot
 * \brief initialize text structure
 * 
 * Set default text data on uninitialized memory.
 * 
 * \param tx   text data
 * \param from copy data template
 */
extern void mpt_text_init(MPT_STRUCT(text) *tx, const MPT_STRUCT(text) *from)
{
	if (from) {
		*tx = *from;
		
		if (from->_font) tx->_font = strdup(from->_font);
		if (from->_value) tx->_value = strdup(from->_value);
		
		return;
	}
	*tx = def_text;
}
/*!
 * \ingroup mptPlot
 * \brief set text properties
 * 
 * Change or reset text structure properties.
 * 
 * \param tx   text data
 * \param name property name
 * \param src  value source
 */
extern int mpt_text_set(MPT_STRUCT(text) *tx, const char *name, MPT_INTERFACE(metatype) *src)
{
	int len;
	
	/* auto-select matching property */
	if (!name) {
		const MPT_STRUCT(text) *from;
		
		if (!src) {
			return MPT_ERROR(BadOperation);
		}
		if ((len = src->_vptr->conv(src, MPT_ENUM(TypeText) | MPT_ENUM(ValueConsume), &from)) >= 0) {
			mpt_text_fini(tx);
			mpt_text_init(tx, from);
			return len ? 1 : 0;
		}
		if ((len = mpt_string_pset(&tx->_value, src)) >= 0) {
			return len;
		}
		if ((len = src->_vptr->conv(src, MPT_ENUM(TypeColor) | MPT_ENUM(ValueConsume), &tx->color)) >= 0) {
			return len ? 1 : 0;
		}
		return MPT_ERROR(BadType);
	}
	/* copy from sibling */
	if (!*name) {
		const MPT_STRUCT(text) *from;
		
		if (!src) {
			mpt_text_fini(tx);
			return 0;
		}
		if ((len = src->_vptr->conv(src, MPT_ENUM(TypeText) | MPT_ENUM(ValueConsume), &from)) >= 0) {
			mpt_text_fini(tx);
			mpt_text_init(tx, from);
			return len ? 1 : 0;
		}
		return MPT_ERROR(BadType);
	}
	/* set properties */
	if (!strcasecmp(name, "value")) {
		if (!src) {
			mpt_string_set(&tx->_value, 0, 0);
			return 0;
		}
		return mpt_string_pset(&tx->_value, src);
	}
	if (!strcasecmp(name, "font")) {
		if (!src) {
			mpt_string_set(&tx->_font, 0, 0);
			return 0;
		}
		return mpt_string_pset(&tx->_font, src);
	}
	if (!strcasecmp(name, "x")) {
		if (!src) {
			tx->pos.x = def_text.pos.x;
			return 0;
		}
		if ((len = src->_vptr->conv(src, 'f' | MPT_ENUM(ValueConsume), &tx->pos.x)) < 0) {
			return len;
		}
		return len ? 1 : 0;
	}
	if (!strcasecmp(name, "y")) {
		if (!src) {
			tx->pos.y = def_text.pos.y;
			return 0;
		}
		if ((len = src->_vptr->conv(src, 'f' | MPT_ENUM(ValueConsume), &tx->pos.y)) < 0) {
			return len;
		}
		return len ? 1 : 0;
	}
	if (!strcasecmp(name, "pos")) {
		int l1, l2;
		float tmp;
		
		if (!src) {
			tx->pos = def_text.pos;
			return 0;
		}
		if ((l1 = src->_vptr->conv(src, 'f' | MPT_ENUM(ValueConsume), &tmp)) < 0) {
			return l1;
		}
		if (!l1) {
			tx->pos = def_text.pos;
			return 0;
		}
		if ((l2 = src->_vptr->conv(src, 'f' | MPT_ENUM(ValueConsume), &tx->pos.y)) < 0) {
			return l2;
		}
		tx->pos.x = tmp;
		return l2 ? 2 : 1;
	}
	if (!strcasecmp(name, "color")) {
		return mpt_color_pset(&tx->color, src);
	}
	if (!strcasecmp(name, "size")) {
		if (!src) {
			tx->size = def_text.size;
			return 0;
		}
		if (!(len = src->_vptr->conv(src, 'y' | MPT_ENUM(ValueConsume), &tx->size))) tx->size = def_text.size;
		return len <= 0 ? len : 1;
	}
	if (!strcasecmp(name, "align")) {
		if (!src) {
			tx->align = def_text.align;
			return 0;
		}
		if (!(len = src->_vptr->conv(src, 'c' | MPT_ENUM(ValueConsume), &tx->align))) tx->align = def_text.align;
		return len <= 0 ? len : 1;
	}
	if (!strcasecmp(name, "angle")) {
		if (!src) {
			tx->angle = def_text.angle;
			return 0;
		}
		if (!(len = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &tx->angle))) tx->angle = def_text.angle;
		return len <= 0 ? len : 1;
	}
	return MPT_ERROR(BadArgument);
}

/*!
 * \ingroup mptPlot
 * \brief get text properties
 * 
 * Get text property parameters.
 * 
 * \param text  text data
 * \param pr    property to query
 * 
 * \return property changed
 */
extern int mpt_text_get(const MPT_STRUCT(text) *tx, MPT_STRUCT(property) *pr)
{
	static const char cfmt[2] = { MPT_ENUM(TypeColor) };
	static const MPT_STRUCT(property) elem[] = {
		{"color",  "text color",     { cfmt, (void *) MPT_offset(text,color) } },
		{"pos",    "text position",  { "ff", (void *) MPT_offset(text, pos) } },
		{"size",   "text size",      { "y",  (void *) MPT_offset(text,size) } },
		{"align",  "text alignment", { "c",  (void *) MPT_offset(text,align) } },
		{"angle",  "text angle",     { "d",  (void *) MPT_offset(text,angle) } },
		{"value",  "text data",      { "s",  (void *) MPT_offset(text,_value) } },
		{"font",   "text font",      { "s",  (void *) MPT_offset(text,_font) } },
	};
	static const MPT_STRUCT(property) elem_xy[] = {
		{"x",  "x start position",  { "f", (void *) MPT_offset(text, pos.x)} },
		{"y",  "y start position",  { "f", (void *) MPT_offset(text, pos.y)} }
	};
	static const char format[] = {
		's', 's',       /* value, font */
		MPT_ENUM(TypeColor),
		'y', 'y', 'y',  /* font style, weight, size, */
		'c',            /* text alignment */
		'f', 'f',       /* position */
		'd',            /* angle */
		0
	};
	int pos;
	
	if (!pr) {
		return MPT_ENUM(TypeText);
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
		pr->name = "text";
		pr->desc = "mpt text data";
		pr->val.fmt = format;
		pr->val.ptr = tx;
		
		return 0;
	}
	/* set position independently */
	else if (!pr->name[1]) {
		if (pr->name[0] == 'x') {
			*pr = elem_xy[0];
			pos = (tx && tx->pos.x == def_text.pos.x) ? 'd' : 0;
		}
		else if (pr->name[0] == 'y') {
			*pr = elem_xy[1];
			pos = (tx && tx->pos.x == def_text.pos.x) ? 'd' : 0;
		}
		else {
			return MPT_ERROR(BadArgument);
		}
		pr->val.ptr = ((uint8_t *) tx) + (intptr_t) pr->val.ptr;
		
		return pos;
	}
	/* find property by name */
	else if ((pos = mpt_property_match(pr->name, -1, elem, MPT_arrsize(elem))) < 0) {
		return pos;
	}
	pr->name = elem[pos].name;
	pr->desc = elem[pos].desc;
	pr->val.fmt = elem[pos].val.fmt;
	/* adjust base address */
	pr->val.ptr = ((uint8_t *) tx) + (intptr_t) elem[pos].val.ptr;
	
	if (!tx) {
		return 0;
	}
	return mpt_value_compare(&pr->val, ((uint8_t *) &def_text) + (intptr_t) elem[pos].val.ptr);
}
