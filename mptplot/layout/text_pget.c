/*!
 * get properties from text structure.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "layout.h"

/* set/get functions */
static int set_pos(float *val, MPT_INTERFACE(source) *src)
{
	int len;
	if (!src) return (*val != 0.5) ? 1 : 0;
	if (!(len = src->_vptr->conv(src, 'f', val))) *val = 0.5;
	return len;
}
static int set_pos2(float *val, MPT_INTERFACE(source) *src, const char **fmt)
{
	int l1, l2;
	float tmp;
	
	*fmt = "ff";
	
	if (!src) {
		l1 = (val[0] == (float) 0.5) ? 0 : 1;
		l2 = (val[1] == (float) 0.5) ? 0 : 2;
		return l1 + l2;
	}
	if ((l1 = src->_vptr->conv(src, 'f', &tmp)) < 0)
		return l1;
	
	if (!l1) {
		val[0] = val[1] = 0.5;
		return 0;
	}
	if ((l2 = src->_vptr->conv(src, 'f', val+1)) < 0)
		return l2;
	
	val[0] = tmp;
	
	if (l2) return l1 + l2;
	
	val[1] = 0.5;
	
	return l1;
}
static int set_size(uint8_t *val, MPT_INTERFACE(source) *src, const char **fmt)
{
	int len;
	*fmt = "y";
	if (!src) return (*val != 10) ? 1 : 0;
	if (!(len = src->_vptr->conv(src, 'y', val))) *val = 10;
	return len;
}
static int set_align(char *val, MPT_INTERFACE(source) *src, const char **fmt)
{
	int len;
	*fmt = "c";
	if (!src) return (*val != '5') ? 1 : 0;
	if (!(len = src->_vptr->conv(src, 'c', val))) *val = 1;
	return len;
}
static int set_angle(double *val, MPT_INTERFACE(source) *src, const char **fmt)
{
	int len;
	*fmt = "d";
	if (!src) return (*val != 0.0) ? 1 : 0;
	if (!(len = src->_vptr->conv(src, 'd', val))) *val = 0.0;
	return len;
}

static int set_text(MPT_STRUCT(text) *txt, MPT_INTERFACE(source) *src)
{
	MPT_STRUCT(text) *tx;
	int len;
	
	if ((len = src->_vptr->conv(src, MPT_ENUM(TypeText), &tx)) >= 0) {
		mpt_text_fini(txt);
		mpt_text_init(txt, tx);
		return len;
	}
	if ((len = mpt_text_pset(&txt->_value, src)) >= 0) {
		return len;
	}
	if ((len = src->_vptr->conv(src, MPT_ENUM(TypeColor), &txt->color)) >= 0) {
		return len;
	}
	errno = ENOTSUP;
	return MPT_ERROR(BadType);
}

/*!
 * \ingroup mptPlot
 * \brief text properties
 * 
 * Get/Set text data elements.
 * 
 * \param text  text data
 * \param pr    property to query
 * \param src   data source to change property
 * 
 * \return consumed/changed value
 */
extern int mpt_text_pget(MPT_STRUCT(text) *text, MPT_STRUCT(property) *pr, MPT_INTERFACE(source) *src)
{
#pragma GCC diagnostic ignored "-Wpedantic"
	static const MPT_STRUCT(property) elem[] = {
		{"color",  "text color",     { (char *) mpt_color_pset, (void *) MPT_offset(text,color) } },
		{"pos",    "text position",  { (char *) set_pos2,       (void *) MPT_offset(text, pos) } },
		{"size",   "text size",      { (char *) set_size,       (void *) MPT_offset(text,size) } },
		{"align",  "text alignment", { (char *) set_align,      (void *) MPT_offset(text,align) } },
		{"angle",  "text angle",     { (char *) set_angle,      (void *) MPT_offset(text,angle) } },
		{"value",  "text data",      { (char *) mpt_text_pset,  (void *) MPT_offset(text,_value) } },
		{"font",   "text font",      { (char *) mpt_text_pset,  (void *) MPT_offset(text,_font) } },
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
	static const char color_fmt[2] = { MPT_ENUM(TypeColor) };
	MPT_STRUCT(property) self;
	int pos, (*set)();
	
	if (!pr) {
		return src ? set_text(text, src) : MPT_ENUM(TypeText);
	}
	self = *pr;
	if (self.name) {
		if (!*self.name) {
			pos = 0;
			if (src && text && (pos = set_text(text, src)) < 0) {
				return pos;
			}
			pr->name = "line";
			pr->desc = "mpt line data";
			pr->val.fmt = format;
			pr->val.ptr = text;
			
			return pos;
		}
		/* set position independently */
		if (!self.name[1]) {
			if (!text) return -1;
			
			if (self.name[0] == 'x') {
				self = elem_xy[0];
			}
			else if (self.name[0] == 'y') {
				self = elem_xy[1];
			}
			else {
				return -1;
			}
			self.val.ptr = ((uint8_t *) text) + (intptr_t) self.val.ptr;
			
			if ((pos = set_pos((void *) self.val.ptr, src)) < 0) {
				return -2;
			}
			*pr = self;
			return pos;
		}
		else if ((pos = mpt_property_match(self.name, -1, elem, MPT_arrsize(elem))) < 0) {
			return pos;
		}
	}
	else if (src) {
		return MPT_ERROR(BadOperation);
	}
	else if ((pos = (intptr_t) pr->desc) < 0 || pos >= (int) MPT_arrsize(elem)) {
		return MPT_ERROR(BadArgument);
	}
	set = (int (*)()) elem[pos].val.fmt;
	self.name = elem[pos].name;
	self.desc = elem[pos].desc;
	self.val.fmt = pos < 1 ? color_fmt : "s"; /* set for external non-modifying handler */
	self.val.ptr = ((uint8_t *) text) + (intptr_t) elem[pos].val.ptr;
	
	if (text && (pos = set(self.val.ptr, src, &self.val.fmt)) < 0) {
		return pos;
	}
	*pr = self;
	return pos;
}
