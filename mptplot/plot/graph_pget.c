/*!
 * get properties from line structure.
 */

#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "plot.h"

static int set_fg(MPT_STRUCT(color) *bg, MPT_INTERFACE(source) *src, const char **fmt)
{
	static const char cfmt[] = { MPT_ENUM(TypeColor) };
	*fmt = cfmt;
	return mpt_color_pset(bg, src);
}
static int set_bg(MPT_STRUCT(color) *bg, MPT_INTERFACE(source) *src, const char **fmt)
{
	static const char cfmt[] = { MPT_ENUM(TypeColor) };
	*fmt = cfmt;
	if (!src) return bg->alpha ? (bg->red || bg->green || bg->blue) : 0;
	return mpt_color_pset(bg, src);
}
static int set_pos(float *val, MPT_INTERFACE(source) *src, const char **fmt)
{
	int l1, l2;
	*fmt = "ff";
	if (!src) return (val[0] != 0.0 || val[1] != 0.0) ? 1 : 0;
	if ((l1 = src->_vptr->conv(src, 'f', val)) < 0) return l1;
	if ((l2 = src->_vptr->conv(src, 'f', val+1)) <= 0) {
		val[1] = val[0]; l2 = 0;
	}
	if (!l1) val[0] = val[1] = 0.0;
	return l1 + l2;
}
static int set_scale(float *val, MPT_INTERFACE(source) *src, const char **fmt)
{
	int l1, l2;
	*fmt = "ff";
	if (!src) return (val[0] != 1.0 || val[1] != 1.0) ? 1 : 0;
	if ((l1 = src->_vptr->conv(src, 'f', val)) < 0) return l1;
	if ((l2 = src->_vptr->conv(src, 'f', val+1)) <= 0) {
		val[1] = val[0]; l2 = 0;
	}
	if (!l1) val[0] = val[1] = 1.0;
	return l1 + l2;
}

static int set_type(uint8_t *val, MPT_INTERFACE(source) *src, const char **fmt)
{
	int len;
	*fmt = "c";
	if (!src) return *val ? 1 : 0;
	if ((len = src->_vptr->conv(src, 'c', val)) < 0) return len;
	if (!len) *val = 0;
	return len;
}

static int set_align(uint8_t *val, MPT_INTERFACE(source) *src, const char **fmt)
{
	static const uint8_t align = MPT_ENUM(AlignBegin) | MPT_ENUM(AlignBegin) << 2 | MPT_ENUM(AlignBegin) << 4;
	const char *v;
	int len;
	
	*fmt = "y";
	
	if (!src) {
		return (align == *val) ? 0 : 1;
	}
	else if ((len = src->_vptr->conv(src, 'y', val)) > 0) {
		return len;
	}
	else if ((len = src->_vptr->conv(src, 's', &v)) < 0) {
		return len;
	}
	else {
		uint8_t n = 0;
		int i = 0;
		
		while (len ? i < len : v[i]) {
			if (i >= 4) {
				break;
			}
			switch (v[i++]) {
			  case 0: len = i - 1; break;
			  case 'B': case 'b': n |= MPT_ENUM(AlignBegin) << i*2; break;
			  case 'E': case 'e': n |= MPT_ENUM(AlignEnd)   << i*2; break;
			  case 'Z': case 'z': n |= MPT_ENUM(AlignZero)  << i*2; break;
			  default:;
			}
		}
		*val = n;
		
		return i;
	}
}

static int set_clip(uint8_t *val, MPT_INTERFACE(source) *src, const char **fmt, const void **data)
{
	static const char clip[][4] = {
		"", "x", "y", "xy",
		"z" "xz", "yz", "xyz"
	};
	const char *v;
	uint8_t nv = 0;
	int len;
	
	if (!src) {
		len = (nv = *val) ? 0 : 1;
	}
	else if ((len = src->_vptr->conv(src, 'y', val)) > 0) {
		return len;
	}
	else if ((len = src->_vptr->conv(src, 's', &v)) < 0) {
		return len;
	}
	else {
		int i = 0;
		
		while (len ? i < len : v[i]) {
			switch (v[i++]) {
			  case 0: len = i - 1; break;
			  case 'x': nv |= 1; break;
			  case 'y': nv |= 2; break;
			  case 'z': nv |= 4; break;
			  default:  nv |= 8;
			}
		}
		len = i;
	}
	if (nv < 8) {
		*fmt = 0;
		*data = clip[nv];
	}
	else {
		*fmt = "y";
		*data = val;
	}
	return len;
}

static int set_graph(MPT_STRUCT(graph) *gr, MPT_INTERFACE(source) *src)
{
	MPT_STRUCT(graph) *from;
	int len;
	
	if ((len = src->_vptr->conv(src, MPT_ENUM(TypeGraph), &from)) >= 0) {
		mpt_graph_fini(gr);
		mpt_graph_init(gr, from);
		return len;
	}
	if ((len = src->_vptr->conv(src, MPT_ENUM(TypeColor), &gr->fg)) > 0) {
		return len;
	}
	if ((len = src->_vptr->conv(src, 'y', &gr->grid)) > 0) {
		return len;
	}
	errno = ENOTSUP;
	return MPT_ERROR(BadType);
}

/*!
 * \ingroup mptPlot
 * \brief graph properties
 * 
 * Get/Set graph data elements.
 * 
 * \param gr  graph data
 * \param pr  property to query
 * \param src data source to change property
 * 
 * \return consumed/changed value
 */
extern int mpt_graph_pget(MPT_STRUCT(graph) *graph, MPT_STRUCT(property) *pr, MPT_INTERFACE(source) *src)
{
#pragma GCC diagnostic ignored "-Wpedantic"
	static const MPT_STRUCT(property) elem[] = {
		{"axes",       "axis names to bind",  { (char *) mpt_text_pset,  (void *) MPT_offset(graph,_axes)} },
		{"worlds",     "world names to bind", { (char *) mpt_text_pset,  (void *) MPT_offset(graph,_worlds)} },
		
		{"foreground", "foreground color",    { (char *) set_fg,         (void *) MPT_offset(graph,fg)} },
		{"background", "background color",    { (char *) set_bg,         (void *) MPT_offset(graph,bg)} },
		
		{"pos",        "origin point",        { (char *) set_pos,        (void *) MPT_offset(graph,pos)} },
		{"scale",      "scale factor",        { (char *) set_scale,      (void *) MPT_offset(graph,scale)} },
		
		{"grid",       "grid type",           { (char *) set_type,       (void *) MPT_offset(graph,grid)} },
		{"align",      "axis alignment",      { (char *) set_align,      (void *) MPT_offset(graph,align)} },
		{"clip",       "clip data display",   { (char *) set_clip,       (void *) MPT_offset(graph,clip)} },
		
		{"lpos",       "legend position",     { (char *) set_type,       (void *) MPT_offset(graph,lpos)} }
	};
	static const char format[] = {
		's', 's',
		MPT_ENUM(TypeColor),
		MPT_ENUM(TypeColor),
		'f', 'f', /* position */
		'f', 'f', /* scaling */
		'y',      /* grid type */
		'y',      /* axis alignment */
		'y',      /* frame type */
		'y',      /* clipping */
		'c',      /* legend alignment */
		0
	};
	MPT_STRUCT(property) self;
	int ret, pos, (*set)();
	
	if (!pr) {
		return (src && graph) ? set_graph(graph, src) : MPT_ENUM(TypeGraph);
	}
	self = *pr;
	if (self.name) {
		if (!*self.name) {
			pos = 0;
			if (src && graph && (pos = set_graph(graph, src)) < 0) {
				return pos;
			}
			pr->name = "graph";
			pr->desc = "mpt graph data";
			pr->val.fmt = format;
			pr->val.ptr = graph;
			
			return pos;
		}
		else if ((pos = mpt_property_match(self.name, 2, elem, MPT_arrsize(elem))) < 0) {
			return pos;
		}
	}
	else if (src) {
		return MPT_ERROR(BadOperation);
	}
	else if ((pos = (intptr_t) pr->desc) < 0 || pos >= (int) MPT_arrsize(elem)) {
		return MPT_ERROR(BadArgument);
	}
	if (!graph) {
		*pr = elem[pos];
		pr->val.fmt = "";
		return pos;
	}
	set = (int (*)()) elem[pos].val.fmt;
	self.name = elem[pos].name;
	self.desc = elem[pos].desc;
	self.val.ptr = ((uint8_t *) graph) + (intptr_t) elem[pos].val.ptr;
	
	if (pos < 2) {
		self.val.fmt = "s";
	}
	if (!graph) {
		*pr = elem[pos];
		pr->val.fmt = self.val.fmt;
		return pos;
	}
	if ((ret = set(self.val.ptr, src, &self.val.fmt, &self.val.ptr)) < 0) {
		return ret;
	}
	*pr = self;
	
	return ret;
}
