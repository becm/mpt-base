/*!
 * get properties from line structure.
 */

#define _POSIX_C_SOURCE 200809L /* for strdup() */

#include <string.h>
#include <strings.h> /* for strcasecmp() */
#include <stdlib.h>
#include <ctype.h>

#include "meta.h"

#include "layout.h"

static const MPT_STRUCT(graph) def_graph = {
	0, 0,      /* _axes, _worlds */
	
	MPT_COLOR_INIT,          /* fg */
	{ 0, 0xff, 0xff, 0xff }, /* bg */
	
	{ 0, 0 },  /* pos */
	{ 1, 1 },  /* scale */
	
	0,  /* grid */
	0,  /* align */
	0,  /* frame */
	0,  /* grid */
	
	0   /* lpos */
};
static const char axes_clip[][4] = {
	"", "x", "y", "xy",
	"z" "xz", "yz", "xyz"
};

/*!
 * \ingroup mptPlot
 * \brief finalize graph
 * 
 * Clear resources of graph data.
 * 
 * \param gr graph data
 */
extern void mpt_graph_fini(MPT_STRUCT(graph) *gr)
{	
	free(gr->_axes);
	free(gr->_worlds);
	*gr = def_graph;
}

/*!
 * \ingroup mptPlot
 * \brief initialize graph structure
 * 
 * Set default values for graph members.
 * 
 * \param gr   uninitialized graph data
 * \param from graph data template
 */
extern void mpt_graph_init(MPT_STRUCT(graph) *gr, const MPT_STRUCT(graph) *from)
{
	if (from) {
		*gr = *from;
		
		if (gr->_axes) gr->_axes = strdup(gr->_axes);
		if (gr->_worlds) gr->_worlds = strdup(gr->_worlds);
		
		return;
	}
	*gr = def_graph;
}
/*!
 * \ingroup mptPlot
 * \brief set graph properties
 * 
 * Change or reset graph structure properties.
 * 
 * \param gr   graph data
 * \param name property name
 * \param src  value source
 */
extern int mpt_graph_set(MPT_STRUCT(graph) *gr, const char *name, MPT_INTERFACE(metatype) *src)
{
	int len = 0;
	
	/* auto-select matching property */
	if (!name) {
		const MPT_STRUCT(graph) *from;
		
		if (!src) {
			return MPT_ERROR(BadOperation);
		}
		if ((len = src->_vptr->conv(src, MPT_ENUM(TypeGraph) | MPT_ENUM(ValueConsume), &from)) >= 0) {
			mpt_graph_fini(gr);
			mpt_graph_init(gr, from);
			return len <= 0 ? len : 1;
		}
		if ((len = src->_vptr->conv(src, MPT_ENUM(TypeColor) | MPT_ENUM(ValueConsume), &gr->fg)) >= 0) {
			return len <= 0 ? len : 1;
		}
		return MPT_ERROR(BadType);
	}
	/* copy from sibling */
	if (!*name) {
		const MPT_STRUCT(graph) *from;
		
		if (!src) {
			mpt_graph_fini(gr);
			return 0;
		}
		if ((len = src->_vptr->conv(src, MPT_ENUM(TypeText) | MPT_ENUM(ValueConsume), &from)) >= 0) {
			mpt_graph_fini(gr);
			mpt_graph_init(gr, from);
			return len <= 0 ? len : 1;
		}
		return MPT_ERROR(BadType);
	}
	if (!strcmp(name, "fg") || !strcasecmp(name, "foreground")) {
		if (!src || !(len = mpt_color_pset(&gr->fg, src))) {
			gr->fg = def_graph.fg;
		}
		return len <= 0 ? len : 1;
	}
	if (!strcmp(name, "bg") || !strcasecmp(name, "background")) {
		if (!src || !(len = mpt_color_pset(&gr->bg, src))) {
			gr->bg = def_graph.bg;
		}
		return len <= 0 ? len : 1;
	}
	if (!strcmp(name, "pos") || !strcasecmp(name, "position")) {
		float val;
		int l1, l2;
		
		if (!src) {
			gr->pos = def_graph.pos;
		}
		if ((l1 = src->_vptr->conv(src, 'f' | MPT_ENUM(ValueConsume), &val)) < 0) {
			return l1;
		}
		if (!l1) {
			gr->pos.y = gr->pos.x = 0.0;
			return 0;
		}
		if (!(l1 & MPT_ENUM(ValueConsume))) {
			gr->pos.y = gr->pos.x = val;
			return 1;
		}
		if ((l2 = src->_vptr->conv(src, 'f' | MPT_ENUM(ValueConsume), &gr->pos.y)) < 0) {
			return l2;
		}
		if (!l2) {
			gr->pos.y = gr->pos.x = val;
		}
		gr->pos.x = val;
		return 2;
	}
	if (!strcasecmp(name, "scale")) {
		float val;
		int l1, l2;
		
		if (!src) {
			gr->scale = def_graph.scale;
		}
		if ((l1 = src->_vptr->conv(src, 'f' | MPT_ENUM(ValueConsume), &val)) < 0) {
			return l1;
		}
		if (!l1) {
			gr->scale.y = gr->scale.x = 1.0;
			return 0;
		}
		if (!(l1 & MPT_ENUM(ValueConsume))) {
			gr->scale.y = gr->scale.x = val;
			return 1;
		}
		if ((l2 = src->_vptr->conv(src, 'f' | MPT_ENUM(ValueConsume), &gr->scale.y)) < 0) {
			return l2;
		}
		if (!l2) {
			gr->scale.y = gr->scale.x = val;
		}
		gr->scale.x = val;
		return 2;
	}
	if (!strcmp(name, "type") || !strcasecmp(name, "gridtype")) {
		if (!src || !(len = src->_vptr->conv(src, 'c', &gr->grid))) {
			gr->grid = def_graph.grid;
		}
		return len <= 0 ? len : 1;
	}
	if (!strcmp(name, "align") || !strcasecmp(name, "alignment")) {
		const char *v;
		uint8_t n = 0;
		int i = 0;
		
		if (!src) {
			gr->align = def_graph.grid;
			return 0;
		}
		if ((len = src->_vptr->conv(src, 'y' | MPT_ENUM(ValueConsume), &gr->align)) >= 0) {
			if (!len) gr->align = def_graph.grid;
			return len <= 0 ? len : 1;
		}
		if ((len = src->_vptr->conv(src, 's' | MPT_ENUM(ValueConsume), &v)) < 0) {
			return len <= 0 ? len : 1;
		}
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
		gr->align = n;
		
		return i;
	}
	if (!strcmp(name, "clip") || !strcasecmp(name, "clipping")) {
		const char *v;
		uint8_t n = 0;
		
		if (!src) {
			gr->clip = def_graph.clip;
			return 0;
		}
		if ((len = src->_vptr->conv(src, 'y' | MPT_ENUM(ValueConsume), &gr->clip)) > 0) {
			return 1;
		}
		if ((len = src->_vptr->conv(src, 's' | MPT_ENUM(ValueConsume), &v)) < 0) {
			return 1;
		}
		if (!len || !v) {
			gr->clip = def_graph.clip;
			return 0;
		}
		while (*v) {
			switch (*v++) {
			  case 'x': n |= 1; break;
			  case 'y': n |= 2; break;
			  case 'z': n |= 4; break;
			  default:  n |= 8;
			}
		}
		gr->clip = n;
		return 1;
	}
	if (!strcmp(name, "lpos")) {
		if (!src || !(len = src->_vptr->conv(src, 'c' | MPT_ENUM(ValueConsume), &gr->lpos))) {
			gr->lpos = def_graph.lpos;
		}
		return len <= 0 ? len : 1;
	}
	if (!strcmp(name, "axes")) {
		if (!src) {
			mpt_string_set(&gr->_axes, 0, 0);
			return 0;
		}
		return mpt_string_pset(&gr->_axes, src);
	}
	if (!strcmp(name, "worlds")) {
		if (!src) {
			mpt_string_set(&gr->_worlds, 0, 0);
			return 0;
		}
		return mpt_string_pset(&gr->_worlds, src);
	}
	return MPT_ERROR(BadArgument);
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
extern int mpt_graph_get(const MPT_STRUCT(graph) *gr, MPT_STRUCT(property) *pr)
{
	static const char cfmt[2] = { MPT_ENUM(TypeColor) };
	static const MPT_STRUCT(property) elem[] = {
		{"axes",       "axis names to bind",  { "s",  (void *) MPT_offset(graph,_axes)} },
		{"worlds",     "world names to bind", { "s",  (void *) MPT_offset(graph,_worlds)} },
		
		{"foreground", "foreground color",    { cfmt, (void *) MPT_offset(graph,fg)} },
		{"background", "background color",    { cfmt, (void *) MPT_offset(graph,bg)} },
		
		{"pos",        "origin point",        { "ff", (void *) MPT_offset(graph,pos)} },
		{"scale",      "scale factor",        { "ff", (void *) MPT_offset(graph,scale)} },
		
		{"grid",       "grid type",           { "y",  (void *) MPT_offset(graph,grid)} },
		{"align",      "axis alignment",      { "y",  (void *) MPT_offset(graph,align)} },
		{"clip",       "clip data display",   { "y",  (void *) MPT_offset(graph,clip)} },
		
		{"lpos",       "legend position",     { "c",  (void *) MPT_offset(graph,lpos)} }
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
	int pos;
	
	if (!pr) {
		return MPT_ENUM(TypeGraph);
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
		pr->name = "graph";
		pr->desc = "mpt graph data";
		pr->val.fmt = format;
		pr->val.ptr = gr;
		
		return 0;
	}
	/* find property by name */
	else if ((pos = mpt_property_match(pr->name, 2, elem, MPT_arrsize(elem))) < 0) {
		return pos;
	}
	if (!gr) {
		*pr = elem[pos];
		pr->val.fmt = "";
		return pos;
	}
	pr->name = elem[pos].name;
	pr->desc = elem[pos].desc;
	pr->val.fmt = elem[pos].val.fmt;
	pr->val.ptr = ((uint8_t *) gr) + (intptr_t) elem[pos].val.ptr;
	
	if (!gr) {
		return 0;
	}
	if (!strcmp(elem[pos].name, "clip")
	    && gr->clip < 8) {
		pr->val.fmt = 0;
		pr->val.ptr = axes_clip[gr->clip];
	}
	return mpt_value_compare(&pr->val, ((uint8_t *) &def_graph) + (intptr_t) elem[pos].val.ptr);
}
