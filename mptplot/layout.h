/*!
 * MPT plotting library
 *  layout data structures
 */

#ifndef _MPT_LAYOUT_H
#define _MPT_LAYOUT_H  @INTERFACE_VERSION@

#ifdef __cplusplus
# include "meta.h"
# include "object.h"
#endif

#include "values.h"

struct iovec;

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(property);

enum MPT_ENUM(AxisFlags) {
	MPT_ENUM(AxisStyleGen)  = 0,
	
	MPT_ENUM(AxisStyleX)    = 0x1,
	MPT_ENUM(AxisStyleY)    = 0x2,
	MPT_ENUM(AxisStyleZ)    = 0x3,
	MPT_ENUM(AxisStyles)    = 0x3,
	
	MPT_ENUM(AxisLimitSwap) = 0x8
};

enum MPT_ENUM(TransformFlags) {
	MPT_ENUM(TransformLimit) = 0x10,
	MPT_ENUM(TransformLg)    = 0x20
};

enum MPT_ENUM(GraphFlags) {
	MPT_ENUM(AlignBegin) = 0x1,
	MPT_ENUM(AlignEnd)   = 0x2,
	MPT_ENUM(AlignZero)  = 0x3
};

enum MPT_ENUM(TextFlags) {
	MPT_ENUM(TextItalic)  = 0x1,
	MPT_ENUM(TextBold)    = 0x2
};

enum MPT_ENUM(LayoutTypes) {
	/* layout data types */
	MPT_ENUM(TypeColor)     = 0x10,  /* DLE */
	MPT_ENUM(TypeLineAttr)  = 0x11,  /* DC1 */
	MPT_ENUM(TypeLine)      = 0x12,  /* DC2 */
	
	/* layout pointer types */
	MPT_ENUM(TypeText)      = 0x14,  /* DC4 */
	MPT_ENUM(TypeAxis)      = 0x15,  /* NAK */
	MPT_ENUM(TypeWorld)     = 0x16,  /* SYN */
	MPT_ENUM(TypeGraph)     = 0x17   /* ETB */
};

/* argb standard color */
MPT_STRUCT(color)
{
#ifdef __cplusplus
	inline color(int r = 0, int g = 0, int b = 0, int a = 255) :
	             alpha(a), red(r), green(g), blue(b)
	{ }
	
	enum { Type = TypeColor };
#else
# define MPT_COLOR_INIT  { 0xff, 0, 0, 0 }
#endif
	uint8_t alpha,  /* alpha part of color */
	        red,    /* red part of color */
	        green,  /* green part of color */
	        blue;   /* blue part of color */
};

/* line (-point symbol) attributes */
MPT_STRUCT(lineattr)
{
#ifdef __cplusplus
	inline lineattr(int s = 1, int w = 1, int sym = 0, int sze = 10) :
	                style(s), width(w), symbol(sym), size(sze)
	{ }
	
	enum { Type = TypeLineAttr };
#else
# define MPT_LINEATTR_INIT  { 1, 1, 0, 10 }
#endif
	uint8_t style,
	        width,   /* line style/width */
	        symbol,
	        size;    /* symbol type/size */
};

/* simple line */
MPT_STRUCT(line)
{
#ifdef __cplusplus
	line();
	
	enum { Type = TypeLine };
#endif
	MPT_STRUCT(color)    color;  /* line colour */
	MPT_STRUCT(lineattr) attr;   /* symbol type/size describe type */
	MPT_STRUCT(fpoint)   from,   /* line start position */
	                     to;     /* line end position */
};

/* parameters for axis display */
MPT_STRUCT(axis)
{
#ifdef __cplusplus
	axis(AxisFlags type = AxisStyleGen);
	axis & operator= (axis const &);
	axis(axis const &);
	~axis();
	
	enum { Type = TypeAxis };
	
	inline const char *title() const
	{
		return _title;
	}
protected:
#endif
	char   *_title;   /* axis title */
#ifdef __cplusplus
public:
#endif
	double  begin,   /* start value */
	        end;     /* end value */
	float   tlen;    /* relative tick length */
	int16_t exp;     /* exponent of axis value scale */
	uint8_t intv,    /* (labeled) interval count */
	        sub,     /* (short) intermediate ticks */
	        format,  /* type flags */
	        dec;     /* decimal places */
	char    lpos,    /* label position/orientation */
	        tpos;    /* title position/orientation */
};

/* format for data lines (and history) */
MPT_STRUCT(world)
{
#ifdef __cplusplus
	world();
	world & operator= (world const &);
	world(world const &);
	~world();
	
	enum { Type = TypeWorld };
	
	inline const char *alias() const
	{
		return _alias;
	}
	bool set_alias(const char *name, int len = -1);
protected:
#endif
	char  *_alias;  /* display name */
#ifdef __cplusplus
public:
#endif
	MPT_STRUCT(color)    color;  /* line color */
	MPT_STRUCT(lineattr) attr;   /* line attributes */
	uint32_t             cyc;    /* fixed size of data buffer */
};

/* graph display parameter */
MPT_STRUCT(graph)
{
#ifdef __cplusplus
	graph();
	graph & operator= (graph const &);
	graph(graph const &);
	~graph();
	
	enum { Type = TypeGraph };
	
	inline const char *axes() const
	{
		return _axes;
	}
	inline const char *worlds() const
	{
		return _worlds;
	}
protected:
#endif
	char             *_axes;
	char             *_worlds;
#ifdef __cplusplus
public:
#endif
	MPT_STRUCT(color)  fg,
	                   bg;    /* fore-/background colour */
	
	MPT_STRUCT(fpoint) pos,
	                   scale; /* position/scale */
	uint8_t            grid,  /* grid type */
	                   align, /* axis align options */
	                   frame, /* legend border type */
	                   clip;  /* no display of out of range data */
	char               lpos;  /* position for legend */
};

/* free form text data */
MPT_STRUCT(text)
{
#ifdef __cplusplus
	text();
	text & operator= (text const &);
	text(const text &);
	~text();
	
	enum { Type = TypeText };
	
	bool set_value(const char *);
	inline const char *value() const
	{
		return _value;
	}
	bool set_font(const char *);
	inline const char *font() const
	{
		return _font;
	}
	
	int set(metatype &);
    protected:
#endif
	char              *_value;   /* text to render */
	char              *_font;    /* selected font */
#ifdef __cplusplus
    public:
#endif
	MPT_STRUCT(color)   color;   /* text color */
	uint8_t             size;    /* text size */
	char                weight,  /* text properties */
	                    style;
	char                align;   /* text alignment */
	MPT_STRUCT(fpoint)  pos;     /* text reference position */
	double              angle;   /* raw text rotation */
};

__MPT_EXTDECL_BEGIN

/* color assignments */
extern int mpt_color_parse(MPT_STRUCT(color) *, const char *);
extern int mpt_color_html (MPT_STRUCT(color) *, const char *);
extern int mpt_color_pset (MPT_STRUCT(color) *, const MPT_INTERFACE(metatype) *);
extern int mpt_color_set(MPT_STRUCT(color) *, int , int , int);
extern int mpt_color_setalpha(MPT_STRUCT(color) *, int);

/* operations on layout elements */
extern void mpt_line_init(MPT_STRUCT(line) *);
extern int  mpt_line_get (const MPT_STRUCT(line) *, MPT_STRUCT(property) *);
extern int  mpt_line_set (MPT_STRUCT(line) *, const char *, const MPT_INTERFACE(metatype) *);

extern void mpt_graph_init(MPT_STRUCT(graph) *, const MPT_STRUCT(graph) *__MPT_DEFPAR(0));
extern void mpt_graph_fini(MPT_STRUCT(graph) *);
extern int  mpt_graph_get (const MPT_STRUCT(graph) *, MPT_STRUCT(property) *);
extern int  mpt_graph_set (MPT_STRUCT(graph) *, const char *, const MPT_INTERFACE(metatype) *);

extern void mpt_axis_init(MPT_STRUCT(axis) *, const MPT_STRUCT(axis) *__MPT_DEFPAR(0));
extern void mpt_axis_fini(MPT_STRUCT(axis) *);
extern int  mpt_axis_get (const MPT_STRUCT(axis) *, MPT_STRUCT(property) *);
extern int  mpt_axis_set (MPT_STRUCT(axis) *, const char *, const MPT_INTERFACE(metatype) *);

extern void mpt_world_init(MPT_STRUCT(world) *, const MPT_STRUCT(world) *__MPT_DEFPAR(0));
extern void mpt_world_fini(MPT_STRUCT(world) *);
extern int  mpt_world_get (const MPT_STRUCT(world) *, MPT_STRUCT(property) *);
extern int  mpt_world_set (MPT_STRUCT(world) *, const char *, const MPT_INTERFACE(metatype) *);

extern void mpt_text_init(MPT_STRUCT(text) *, const MPT_STRUCT(text) *__MPT_DEFPAR(0));
extern void mpt_text_fini(MPT_STRUCT(text) *);
extern int  mpt_text_get (const MPT_STRUCT(text) *, MPT_STRUCT(property) *);
extern int  mpt_text_set (MPT_STRUCT(text) *, const char *, const MPT_INTERFACE(metatype) *);

/* set axis type and lenth */
extern void mpt_axis_setx(MPT_STRUCT(axis) *, double);
extern void mpt_axis_sety(MPT_STRUCT(axis) *, double);
extern void mpt_axis_setz(MPT_STRUCT(axis) *, double);

/* assign line attributes */
extern int mpt_lattr_style (MPT_STRUCT(lineattr) *, const MPT_INTERFACE(metatype) *);
extern int mpt_lattr_width (MPT_STRUCT(lineattr) *, const MPT_INTERFACE(metatype) *);
extern int mpt_lattr_symbol(MPT_STRUCT(lineattr) *, const MPT_INTERFACE(metatype) *);
extern int mpt_lattr_size  (MPT_STRUCT(lineattr) *, const MPT_INTERFACE(metatype) *);
extern int mpt_lattr_set(MPT_STRUCT(lineattr) *, int , int , int , int);

/* change string data */
extern int mpt_string_pset(char **, const MPT_INTERFACE(metatype) *);
extern int mpt_string_set(char **, const char *, int __MPT_DEFPAR(-1));

/* create pline representation for axis ticks */
extern void mpt_ticks_linear(MPT_STRUCT(dpoint) *, size_t , double , double);
extern double mpt_tick_log10(int);

__MPT_EXTDECL_END

#ifdef __cplusplus
class Parse;
struct parseflg;
struct path;

template <> inline __MPT_CONST_TYPE int typeinfo<color>::id()
{
	return color::Type;
}
template <> inline __MPT_CONST_TYPE int typeinfo<lineattr>::id()
{
	return lineattr::Type;
}
template <> inline __MPT_CONST_TYPE int typeinfo<line>::id()
{
	return line::Type;
}
template <> inline __MPT_CONST_TYPE int typeinfo<axis>::id()
{
	return axis::Type;
}
template <> inline __MPT_CONST_TYPE int typeinfo<world>::id()
{
	return world::Type;
}
template <> inline __MPT_CONST_TYPE int typeinfo<graph>::id()
{
	return graph::Type;
}
template <> inline __MPT_CONST_TYPE int typeinfo<text>::id()
{
	return text::Type;
}

template <typename S>
void apply(point<S> *d, const linepart &pt, const S *src, const value_apply &v)
{
	S mx(v.to.x * v.add);
	S my(v.to.y * v.add);
	for (size_t i = 0; i < pt.usr; ++i) {
		d->x += mx;
		d->y += my;
	}
	const point<S> scale(v.to.x * v.scale, v.to.y * v.scale);
	apply<point<S>, S>(d, pt, src, scale);
}
extern void apply_log(point<double> *, const linepart &, const double *, const transform &);



/*! Group implementation using reference array */
class collection : public metatype, public group
{
public:
	virtual ~collection();
	
	void unref() __MPT_OVERRIDE;
	int conv(int, void *) const __MPT_OVERRIDE;
	collection *clone() const __MPT_OVERRIDE;
	
	const class item<metatype> *item(size_t) const __MPT_OVERRIDE;
	class item<metatype> *append(metatype *) __MPT_OVERRIDE;
	size_t clear(const reference * = 0) __MPT_OVERRIDE;
	bool bind(const relation &, logger * = logger::defaultInstance()) __MPT_OVERRIDE;
protected:
	item_array<metatype> _items;
};

/*! Represent elements in layout file */
class layout : public collection
{
public:
	class line;
	class text;
	
	class graph;
	
	layout();
	~layout() __MPT_OVERRIDE;
	
	int property(struct property *) const __MPT_OVERRIDE;
	int set_property(const char *, const metatype *) __MPT_OVERRIDE;
	
	bool bind(const relation &, logger * = logger::defaultInstance()) __MPT_OVERRIDE;
	
	virtual bool load(logger * = logger::defaultInstance());
	virtual bool open(const char *);
	virtual bool reset();
	
	inline span<const class item<graph> > graphs() const
	{
		return _graphs.elements();
	}
	virtual bool set_alias(const char *, int = -1);
	inline const char *alias() const
	{
		return _alias;
	}
	virtual bool set_font(const char *, int = -1);
	inline const char *font() const
	{
		return _font;
	}
	
	fpoint minimal_scale() const;
protected:
	item_array<graph> _graphs;
	Parse *_parse;
	char *_alias;
	char *_font;
};

class layout::line : public metatype, public object, public ::mpt::line
{
public:
	line(const ::mpt::line *from = 0);
	virtual ~line();
	
	void unref() __MPT_OVERRIDE;
	int conv(int, void *) const __MPT_OVERRIDE;
	
	int property(struct property *) const __MPT_OVERRIDE;
	int set_property(const char *, const metatype * = 0) __MPT_OVERRIDE;
};

class layout::text : public metatype, public object, public ::mpt::text
{
public:
	text(const ::mpt::text *from = 0);
	virtual ~text();
	
	void unref() __MPT_OVERRIDE;
	int conv(int, void *) const __MPT_OVERRIDE;
	
	int property(struct property *) const __MPT_OVERRIDE;
	int set_property(const char *, const metatype *) __MPT_OVERRIDE;
};

/*! Container and binding for data to axes */
class layout::graph : public collection, public ::mpt::graph
{
public:
	class axis;
	class world;
	class transform;
	class data : public reference
	{
	public:
		data(class world *w = 0);
		virtual ~data()
		{ }
		
		void unref() __MPT_OVERRIDE;
		
		reference_wrapper<class world> world;
		reference_wrapper<class cycle> cycle;
	};
	
	graph(const ::mpt::graph * = 0);
	~graph() __MPT_OVERRIDE;
	
	int conv(int, void *) const __MPT_OVERRIDE;
	
	int property(struct property *) const __MPT_OVERRIDE;
	int set_property(const char *, const metatype *) __MPT_OVERRIDE;
	
	bool bind(const relation &from, logger * = logger::defaultInstance()) __MPT_OVERRIDE;
	
	virtual class item<axis> *add_axis(axis * = 0, const char * = 0, int = -1);
	inline span<const class item<axis> > axes() const
	{
		return _axes.elements();
	}
	virtual class item<data> *add_world(world * = 0, const char * = 0, int = -1);
	inline span<const class item<data> > worlds() const
	{
		return _worlds.elements();
	}
	virtual bool set_cycle(int , const reference_wrapper<class cycle> &) const;
	virtual const reference_wrapper<class cycle> *cycle(int) const;
	
	const class ::mpt::transform &transform();
	
	const struct value_apply *transform_part(int = -1) const;
	int transform_flags(int = -1) const;
	
	bool update_transform(int dim = -1);
	
protected:
	reference_wrapper<class transform> _gtr;
	item_array<axis> _axes;
	item_array<data> _worlds;
};

/*! Transformation parameters/interface for (up to) 3 dimensions */
class layout::graph::transform : public ::mpt::transform
{
public:
	struct data : public value_apply
	{
		data(int = -1);
		
		struct range limit;
	};
	
	transform();
	~transform() __MPT_OVERRIDE;
	
	int dimensions() const __MPT_OVERRIDE;
	
	linepart part(unsigned , const double *, int) const __MPT_OVERRIDE;
	bool apply(unsigned , const linepart &pt, point<double> *, const double *) const __MPT_OVERRIDE;
	point<double> zero() const __MPT_OVERRIDE;
	
	data   _dim[3];
	fpoint _base;
};

class layout::graph::axis : public metatype, public object, public ::mpt::axis
{
public:
	axis(const ::mpt::axis *from = 0);
	axis(AxisFlags type);
	virtual ~axis();
	
	void unref() __MPT_OVERRIDE;
	int conv(int, void *) const __MPT_OVERRIDE;
	
	int property(struct property *) const __MPT_OVERRIDE;
	int set_property(const char *, const metatype *) __MPT_OVERRIDE;
};

class layout::graph::world : public metatype, public object, public ::mpt::world
{
public:
	world(int = 0);
	world(const ::mpt::world *);
	virtual ~world();
	
	void unref() __MPT_OVERRIDE;
	int conv(int, void *) const __MPT_OVERRIDE;
	
	int property(struct property *) const __MPT_OVERRIDE;
	int set_property(const char *, const metatype *) __MPT_OVERRIDE;
};

#endif

__MPT_NAMESPACE_END

#endif /* _MPT_LAYOUT_H */

