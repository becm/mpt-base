/*!
 * get properties from line structure.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "plot.h"

/* set/get functions */
static int set_pos(float *val, MPT_INTERFACE(source) *src)
{
	int len;
	if (!src) return (*val != 0.0) ? 1 : 0;
	if (!(len = src->_vptr->conv(src, 'f', val))) *val = 0.0;
	return len;
}

static int set_line(MPT_STRUCT(line) *line, MPT_INTERFACE(source) *src)
{
	int len;
	
	if ((len = src->_vptr->conv(src, MPT_ENUM(TypeLine), line)) >= 0) {
		return len;
	}
	if ((len = src->_vptr->conv(src, MPT_ENUM(TypeColor), &line->color)) >= 0)
		return len;
	
	if ((len = src->_vptr->conv(src, MPT_ENUM(TypeLineAttr), &line->attr)) >= 0)
		return len;
	
	errno = ENOTSUP;
	return -1;
}

/*!
 * \ingroup mptPlot
 * \brief line properties
 * 
 * Get/Set line data elements.
 * 
 * \param line  line data
 * \param pr    property to query
 * \param src   data source to change property
 * 
 * \return consumed/changed value
 */
extern int mpt_line_pget(MPT_STRUCT(line) *line, MPT_STRUCT(property) *pr, MPT_INTERFACE(source) *src)
{
	static const MPT_STRUCT(property) elem[] = {
		{"color",	"line color",	(char *) mpt_color_pset,	(void *) MPT_offset(line,color)		},
		{"x1",		"line start",	(char *) set_pos,		(void *) MPT_offset(line,from.x)	},
		{"x2",		"line end (x)",	(char *) set_pos,		(void *) MPT_offset(line,to.x)		},
		{"y1",		"line start (y)", (char *) set_pos,		(void *) MPT_offset(line,from.y)	},
		{"y2",		"line end (y)",	(char *) set_pos,		(void *) MPT_offset(line,to.y)		},
		{"width",	"line width",	(char *) mpt_lattr_width,	(void *) MPT_offset(line,attr.width)	}, /* pass line::attr to setter */
		{"style",	"line style",	(char *) mpt_lattr_style,	(void *) MPT_offset(line,attr.style)	},
		{"symbol",	"symbol type",	(char *) mpt_lattr_symbol,	(void *) MPT_offset(line,attr.symbol)	},
		{"size",	"symbol size",	(char *) mpt_lattr_size,	(void *) MPT_offset(line,attr.size)	}
	};
	static const char format[] = {
		MPT_ENUM(TypeColor),
		MPT_ENUM(TypeLineAttr),
		'f', 'f', 'f', 'f',
		0
	};
	MPT_STRUCT(property) self;
	int	pos, (*set)();
	
	if (!pr) {
		return src ? set_line(line, src) : MPT_ENUM(TypeLine);
	}
	self = *pr;
	if (self.name) {
		if (!*self.name) {
			pos = 0;
			if (src && line && (pos = set_line(line, src)) < 0) {
				return -2;
			}
			pr->name = "line";
			pr->desc = "mpt line data";
			pr->fmt  = format;
			pr->data = line;
			
			return pos;
		}
		else if ((pos = mpt_property_match(self.name, -1, elem, MPT_arrsize(elem))) < 0) {
			return -1;
		}
	}
	else if (src) {
		return -3;
	}
	else if ((pos = (intptr_t) pr->desc) < 0 || pos >= (int) MPT_arrsize(elem)) {
		return -1;
	}
	set = (int (*)()) elem[pos].fmt;
	self.name = elem[pos].name;
	self.desc = elem[pos].desc;
	self.data = ((uint8_t *) line) + (intptr_t) elem[pos].data;
	
	if (pos < 1) {
		self.fmt = "#";
	}
	else if (pos < 5) {
		self.fmt = "g";
	}
	else {
		self.fmt = "C";
		if (line && (pos = set(&line->attr, src)) < 0) return -2;
		*pr = self;
		return pos;
	}
	if (line && (pos = set(self.data, src)) < 0) return -2;
	*pr = self;
	return pos;
}
