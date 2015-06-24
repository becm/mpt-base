/*!
 * get parameter from axis structure.
 */

#include <string.h>
#include <errno.h>

#include "plot.h"

/* modification functions */
static int set_begin(double *val, MPT_INTERFACE(source) *src, const char **fmt)
{
	int len;
	*fmt = "D";
	if (!src) return (*val != 0.0) ? 1 : 0;
	if (!(len = src->_vptr->conv(src, 'd', val))) *val = 0.0;
	return len;
}
static int set_end(double *val, MPT_INTERFACE(source) *src, const char **fmt)
{
	int len;
	*fmt = "D";
	if (!src) return (*val != 1.0) ? 1 : 0;
	if (!(len = src->_vptr->conv(src, 'd', val))) *val = 1.0;
	return len;
}
static int set_tlen(float *val, MPT_INTERFACE(source) *src, const char **fmt)
{
	float tlen;
	int   len;
	
	*fmt = "F";
	if (!src) return (*val == 0.3f) ? 0 : 1;
	
	if ((len = src->_vptr->conv(src, 'f', &tlen)) > 0) {
		*val = tlen;
	}
	return len;
}
static int set_intv(MPT_STRUCT(axis) *ax, MPT_INTERFACE(source) *src, const char **fmt, const void **dat)
{
	static const char desc[] = "log\0";
	const char *l;
	int len;
	
	if (!src) {
		if (ax->format & MPT_ENUM(AxisLg)) {
			*fmt = 0;
			*dat = desc;
			return 3;
		}
		*fmt = "B";
		*dat = &ax->intv;
		return ax->intv;
	}
	if (!(len = src->_vptr->conv(src, 'B', &ax->intv))) {
		ax->format &= ~MPT_ENUM(AxisLg);
		ax->intv = 0;
	}
	if (len >= 0 || (len = src->_vptr->conv(src, 's', &l)) < 0 || len < 0 || !l) {
		*fmt = "B";
		*dat = &ax->intv;
		ax->format &= ~MPT_ENUM(AxisLg);
		return len;
	}
	if (!strncasecmp(l, desc, 3)) {
		ax->format |= MPT_ENUM(AxisLg);
		ax->intv = 0;
		*fmt = 0;
		*dat = desc;
		return len;
	}
	return -3;
}
static int set_exp(int16_t *val, MPT_INTERFACE(source) *src, const char **fmt)
{
	int len;
	*fmt = "h";
	if (!src) return (*val != 0) ? (*val < 1 ? 2 : 1) : 0;
	if (!(len = src->_vptr->conv(src, 'h', val))) *val = 0;
	return len;
}
static int set_sub(int8_t *val, MPT_INTERFACE(source) *src, const char **fmt)
{
	int len;
	*fmt = "b";
	if (!src) return *val;
	if (!(len = src->_vptr->conv(src, 'b', val))) *val = 0;
	return len;
}
static int set_decimals(uint8_t *val, MPT_INTERFACE(source) *src, const char **fmt)
{
	int len;
	*fmt = "B";
	if (!src) return *val;
	if (!(len = src->_vptr->conv(src, 'B', val))) *val = 0;
	return len;
}
static int set_direction(char *val, MPT_INTERFACE(source) *src, const char **fmt)
{
	const char *s;
	int len;
	
	*fmt = "c";
	
	if (!src) {
		return *val ? 1 : 0;
	}
	if ((len = src->_vptr->conv(src, 's', &s)) >= 0) {
		*val = s ? *s : 0;
		return 1;
	}
	if ((len = src->_vptr->conv(src, 'c', val)) >= 0) {
		return len;
	}
	return len;
}
static int setAxis(MPT_STRUCT(axis) *ax, MPT_INTERFACE(source) *src)
{
	MPT_STRUCT(axis) *from;
	int len;
	if ((len = src->_vptr->conv(src, MPT_ENUM(TypeAxis), &from)) >= 0) {
		mpt_axis_fini(ax);
		mpt_axis_init(ax, from);
		return len;
	}
	errno = ENOTSUP;
	return -1;
}

/*!
 * \ingroup mptPlot
 * \brief axis properties
 * 
 * Get/Set axis data elements.
 * 
 * \param axis axis data
 * \param pr   property to query
 * \param src  data source to change property
 * 
 * \return consumed/changed value
 */
extern int mpt_axis_pget(MPT_STRUCT(axis) *axis, MPT_STRUCT(property) *pr, MPT_INTERFACE(source) *src)
{
	static const MPT_STRUCT(property) elem[] = {
		{"title",     "axis title",              { (char *) mpt_text_pset,  (void *) MPT_offset(axis, _title)} },
		{"begin",     "axis start value",        { (char *) set_begin,      (void *) MPT_offset(axis, begin)} },
		{"tlen",      "relative tick length ",   { (char *) set_tlen,       (void *) MPT_offset(axis, tlen)} },
		{"end",       "axis end value",          { (char *) set_end,        (void *) MPT_offset(axis, end)} },
		{"exponent",  "label exponent",          { (char *) set_exp,        (void *) MPT_offset(axis, exp)} },
		{"intervals", "intervals between ticks", { (char *) set_intv,       0} },
		{"subtick",   "intermediate ticks",      { (char *) set_sub,        (void *) MPT_offset(axis, sub)} },
		{"decimals",  "decimal places",          { (char *) set_decimals,   (void *) MPT_offset(axis, dec)} },
		{"lpos",      "label direction",         { (char *) set_direction,  (void *) MPT_offset(axis, lpos)} },
		{"tpos",      "title direction",         { (char *) set_direction,  (void *) MPT_offset(axis, tpos)} },
	};
	static const char format[] = {
		's',
		'd', 'd', /* axis range values */
		'f',      /* relative tick length */
		'h',      /* scale value */
		'B', 'B', /* intervals, subintervals */
		'B',      /* axis flags */
		'B',      /* decimals */
		'c', 'c', /* label/title direction */
		0
	};
	MPT_STRUCT(property) self = *pr;
	int pos = 0, (*set)(), len;
	
	if (!pr) {
		return src ? setAxis(axis, src) : MPT_ENUM(TypeAxis);
	}
	self = *pr;
	if (self.name) {
		if (!*self.name) {
			pos = 0;
			if (src && axis && (pos = setAxis(axis, src)) < 0) {
				return -2;
			}
			pr->name = "axis";
			pr->desc = "mpt axis data";
			pr->val.fmt = format;
			pr->val.ptr = axis;
			
			return pos;
		}
		else if ((pos = mpt_property_match(self.name, 3, elem, MPT_arrsize(elem))) < 0) {
			return -1;
		}
	}
	else if (src) {
		return -3;
	}
	else if ((pos = (intptr_t) pr->desc) < 0 || pos >= (int) MPT_arrsize(elem)) {
		return -1;
	}
	set = (int(*)()) elem[pos].val.fmt;
	self.name = elem[pos].name;
	self.desc = elem[pos].desc;
	self.val.ptr = ((char *) axis) + (intptr_t) elem[pos].val.ptr;
	
	if (!axis) {
		*pr = elem[pos];
		pr->val.fmt = "";
		return pos;
	}
	if ((len = set(self.val.ptr, src, &self.val.fmt, &self.val.ptr)) < 0) {
		return -2;
	}
	if (pos < 1) {
		self.val.fmt = 0;
		self.val.ptr = axis->_title;
	}
	*pr = self;
	return len;
}
