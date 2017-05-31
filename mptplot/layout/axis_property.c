/*!
 * get parameter from axis structure.
 */

#define _POSIX_C_SOURCE 200809L /* need for strdup() */

#include <string.h>
#include <strings.h> /* for strcasecmp() */
#include <stdlib.h>

#include "meta.h"

#include "layout.h"

static const MPT_STRUCT(axis) def_axis = {
	0,       /* _title */
	0, 1,    /* begin, end */
	0.3,     /* tlen */
	0,       /* exp */
	0,       /* intv */
	0,       /* sub */
	0,       /* format */
	0,       /* dec */
	0, 0     /* lpos, tpos */
};

/*!
 * \ingroup mptPlot
 * \brief finalize axis
 * 
 * Clear resources of axis data.
 * 
 * \param axis axis data
 */
extern void mpt_axis_fini(MPT_STRUCT(axis) *ax)
{	
	free(ax->_title);
	*ax = def_axis;
}

/*!
 * \ingroup mptPlot
 * \brief initialize axis
 * 
 * Set default values for axis members.
 * 
 * \param axis uninitialized axis data
 * \param from axis data template
 */
extern void mpt_axis_init(MPT_STRUCT(axis) *ax, const MPT_STRUCT(axis) *from)
{
	if (from) {
		*ax = *from;
		
		if (ax->_title) {
			ax->_title = strdup(from->_title);
		}
		return;
	}
	*ax = def_axis;
}

static int setPosition(char *val, const MPT_INTERFACE(metatype) *src, int def)
{
	const char *s;
	int len;
	
	if (!src) {
		*val = def;
		return 0;
	}
	if (!(len = src->_vptr->conv(src, 'k', &s))) {
		*val = def;
		return 0;
	}
	if (len < 0
	    || (len = src->_vptr->conv(src, 'c', val)) < 0) {
		return len;
	}
	return 0;
}
/*!
 * \ingroup mptPlot
 * \brief set axis properties
 * 
 * Change or reset axis structure properties.
 * 
 * \param ax   axis data
 * \param name property name
 * \param src  value source
 */
extern int mpt_axis_set(MPT_STRUCT(axis) *ax, const char *name, const MPT_INTERFACE(metatype) *src)
{
	int len;
	
	/* auto-select matching property */
	if (!name) {
		MPT_STRUCT(axis) *from;
		
		if (!src) {
			return MPT_ERROR(BadOperation);
		}
		if ((len = src->_vptr->conv(src, MPT_ENUM(TypeAxis), &from)) >= 0) {
			mpt_axis_fini(ax);
			mpt_axis_init(ax, from);
			return len ? 1 : 0;
		}
		if ((len = mpt_string_pset(&ax->_title, src)) >= 0) {
			return len;
		}
		return MPT_ERROR(BadType);
	}
	/* copy from sibling */
	if (!*name) {
		MPT_STRUCT(axis) *from;
		
		if (!src) {
			mpt_axis_fini(ax);
			return 0;
		}
		if ((len = src->_vptr->conv(src, MPT_ENUM(TypeText), &from)) >= 0) {
			mpt_axis_fini(ax);
			mpt_axis_init(ax, from);
			return len ? 1 : 0;
		}
		return MPT_ERROR(BadType);
	}
	if (!strcasecmp(name, "title")) {
		if (!src) {
			mpt_string_set(&ax->_title, 0, 0);
			return 0;
		}
		return mpt_string_pset(&ax->_title, src);
	}
	if (!strcasecmp(name, "begin")) {
		if (!src) {
			ax->begin = def_axis.begin;
			return 0;
		}
		if (!(len = src->_vptr->conv(src, 'd', &ax->begin))) ax->begin = def_axis.begin;
		return len <= 0 ? len : 1;
	}
	if (!strcasecmp(name, "end")) {
		if (!src) {
			ax->end = def_axis.end;
			return 0;
		}
		if (!(len = src->_vptr->conv(src, 'd', &ax->end))) ax->end = def_axis.end;
		return len <= 0 ? len : 1;
	}
	if (!strcasecmp(name, "tlen")) {
		if (!src) {
			ax->tlen = def_axis.tlen;
			return 0;
		}
		if (!(len = src->_vptr->conv(src, 'f', &ax->tlen))) ax->tlen = def_axis.tlen;
		return len <= 0 ? len : 1;
	}
	if (!strcasecmp(name, "int") || !strcasecmp(name, "intv") || !strcasecmp(name, "intervals")) {
		const char *l;
		if (!src) {
			ax->intv = def_axis.intv;
			ax->format &= ~MPT_ENUM(AxisLg);
			return 0;
		}
		if (!(len = src->_vptr->conv(src, 'y', &ax->intv))) {
			ax->format &= ~MPT_ENUM(AxisLg);
			ax->intv = 0;
		}
		if (len >= 0 || (len = src->_vptr->conv(src, 's', &l)) < 0 || len < 0 || !l) {
			ax->format &= ~MPT_ENUM(AxisLg);
		}
		else if (!strncasecmp(l, "log", 3)) {
			ax->format |= MPT_ENUM(AxisLg);
			ax->intv = 0;
		}
		return len <= 0 ? len : 1;
	}
	if (!strcasecmp(name, "exp") || !strcasecmp(name, "exponent")) {
		if (!src) {
			ax->exp = def_axis.exp;
			return 0;
		}
		if (!(len = src->_vptr->conv(src, 'n', &ax->exp))) ax->exp = def_axis.exp;
		return len <= 0 ? len : 1;
	}
	if (!strcasecmp(name, "sub") || !strcasecmp(name, "subtick")) {
		if (!src) {
			ax->sub = def_axis.sub;
			return 0;
		}
		if (!(len = src->_vptr->conv(src, 'b', &ax->sub))) ax->sub = def_axis.sub;
		return len <= 0 ? len : 1;
	}
	if (!strcasecmp(name, "dec") || !strcasecmp(name, "decimals")) {
		if (!src) {
			ax->dec = def_axis.dec;
			return 0;
		}
		if (!(len = src->_vptr->conv(src, 'b', &ax->dec))) ax->dec = def_axis.dec;
		return len <= 0 ? len : 1;
	}
	if (!strcasecmp(name, "lpos") || !strcasecmp(name, "labelpos") || !strcasecmp(name, "label position")) {
		return setPosition(&ax->lpos, src, def_axis.lpos);
	}
	if (!strcasecmp(name, "tpos") || !strcasecmp(name, "titlepos") || !strcasecmp(name, "title position")) {
		return setPosition(&ax->tpos, src, def_axis.tpos);
	}
	return MPT_ERROR(BadArgument);
}

/*!
 * \ingroup mptPlot
 * \brief get axis properties
 * 
 * Get axis property parameters.
 * 
 * \param axis axis data
 * \param pr   property to query
 * 
 * \return property id
 */
extern int mpt_axis_get(const MPT_STRUCT(axis) *ax, MPT_STRUCT(property) *pr)
{
	static const MPT_STRUCT(property) elem[] = {
		{"title",     "axis title",              { "s",  (void *) MPT_offset(axis, _title)} },
		{"begin",     "axis start value",        { "d",  (void *) MPT_offset(axis, begin)} },
		{"end",       "axis end value",          { "d",  (void *) MPT_offset(axis, end)} },
		{"tlen",      "relative tick length ",   { "f",  (void *) MPT_offset(axis, tlen)} },
		{"exponent",  "label exponent",          { "n",  (void *) MPT_offset(axis, exp)} },
		{"intervals", "intervals between ticks", { "y",  (void *) MPT_offset(axis, intv)} },
		{"subtick",   "intermediate ticks",      { "y",  (void *) MPT_offset(axis, sub)} },
		{"decimals",  "decimal places",          { "y",  (void *) MPT_offset(axis, dec)} },
		{"lpos",      "label direction",         { "c",  (void *) MPT_offset(axis, lpos)} },
		{"tpos",      "title direction",         { "c",  (void *) MPT_offset(axis, tpos)} },
	};
	static const char format[] = {
		's',
		'd', 'd', /* axis range values */
		'f',      /* relative tick length */
		'n',      /* scale value */
		'y', 'y', /* intervals, subintervals */
		'y',      /* axis flags */
		'y',      /* decimals */
		'c', 'c', /* label/title direction */
		0
	};
	int pos;
	
	if (!pr) {
		return MPT_ENUM(TypeAxis);
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
		pr->name = "axis";
		pr->desc = "mpt axis data";
		pr->val.fmt = format;
		pr->val.ptr = ax;
		
		return MPT_ENUM(TypeAxis);
	}
	/* find property by name */
	else if ((pos = mpt_property_match(pr->name, 3, elem, MPT_arrsize(elem))) < 0) {
		return MPT_ERROR(BadArgument);
	}
	pr->name = elem[pos].name;
	pr->desc = elem[pos].desc;
	
	if (!ax) {
		pr->val = elem[pos].val;
		return 0;
	}
	
	if ((pos == 5) && (ax->format & MPT_ENUM(AxisLg))) {
		static const char desc[] = "log\0";
		pr->val.fmt = 0;
		pr->val.ptr = desc;
	} else {
		pr->val.fmt = elem[pos].val.fmt;
		pr->val.ptr = ((uint8_t *) ax) + (intptr_t) elem[pos].val.ptr;
	}
	return mpt_value_compare(&pr->val, ((uint8_t *) &def_axis) + (intptr_t) elem[pos].val.ptr);
}
