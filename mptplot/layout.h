/*!
 * MPT plotting library
 *  layout data structures
 */

#ifndef _MPT_LAYOUT_H
#define _MPT_LAYOUT_H  @INTERFACE_VERSION@

#ifdef __cplusplus
# include "config.h"
# include "object.h"
#endif

#include "collection.h"
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
	MPT_ENUM(TypeColor)       = 0x10,  /* DLE */
	MPT_ENUM(TypeLineAttr)    = 0x11,  /* DC1 */
	MPT_ENUM(TypeFloatPoint)  = 0x12,  /* DC2 */
	MPT_ENUM(TypeLine)        = 0x13,  /* DC3 */
	
	/* layout pointer types */
	MPT_ENUM(TypeTextPtr)   = 0x14,  /* DC4 */
	MPT_ENUM(TypeAxisPtr)   = 0x15,  /* NAK */
	MPT_ENUM(TypeWorldPtr)  = 0x16,  /* SYN */
	MPT_ENUM(TypeGraphPtr)  = 0x17   /* ETB */
};

/* argb standard color */
MPT_STRUCT(color)
{
#ifdef __cplusplus
	inline color(int r = 0, int g = 0, int b = 0, int a = 255) :
	             alpha(a), red(r), green(g), blue(b)
	{ }
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
extern int mpt_color_pset (MPT_STRUCT(color) *, MPT_INTERFACE(convertable) *);
extern int mpt_color_set(MPT_STRUCT(color) *, int , int , int);
extern int mpt_color_setalpha(MPT_STRUCT(color) *, int);

/* operations on layout elements */
extern void mpt_line_init(MPT_STRUCT(line) *);
extern int  mpt_line_get (const MPT_STRUCT(line) *, MPT_STRUCT(property) *);
extern int  mpt_line_set (MPT_STRUCT(line) *, const char *, MPT_INTERFACE(convertable) *);

extern void mpt_graph_init(MPT_STRUCT(graph) *, const MPT_STRUCT(graph) *__MPT_DEFPAR(0));
extern void mpt_graph_fini(MPT_STRUCT(graph) *);
extern int  mpt_graph_get (const MPT_STRUCT(graph) *, MPT_STRUCT(property) *);
extern int  mpt_graph_set (MPT_STRUCT(graph) *, const char *, MPT_INTERFACE(convertable) *);

extern void mpt_axis_init(MPT_STRUCT(axis) *, const MPT_STRUCT(axis) *__MPT_DEFPAR(0));
extern void mpt_axis_fini(MPT_STRUCT(axis) *);
extern int  mpt_axis_get (const MPT_STRUCT(axis) *, MPT_STRUCT(property) *);
extern int  mpt_axis_set (MPT_STRUCT(axis) *, const char *, MPT_INTERFACE(convertable) *);

extern void mpt_world_init(MPT_STRUCT(world) *, const MPT_STRUCT(world) *__MPT_DEFPAR(0));
extern void mpt_world_fini(MPT_STRUCT(world) *);
extern int  mpt_world_get (const MPT_STRUCT(world) *, MPT_STRUCT(property) *);
extern int  mpt_world_set (MPT_STRUCT(world) *, const char *, MPT_INTERFACE(convertable) *);

extern void mpt_text_init(MPT_STRUCT(text) *, const MPT_STRUCT(text) *__MPT_DEFPAR(0));
extern void mpt_text_fini(MPT_STRUCT(text) *);
extern int  mpt_text_get (const MPT_STRUCT(text) *, MPT_STRUCT(property) *);
extern int  mpt_text_set (MPT_STRUCT(text) *, const char *, MPT_INTERFACE(convertable) *);

/* set axis type and lenth */
extern void mpt_axis_setx(MPT_STRUCT(axis) *, double);
extern void mpt_axis_sety(MPT_STRUCT(axis) *, double);
extern void mpt_axis_setz(MPT_STRUCT(axis) *, double);

/* assign line attributes */
extern int mpt_lattr_style (MPT_STRUCT(lineattr) *, MPT_INTERFACE(convertable) *);
extern int mpt_lattr_width (MPT_STRUCT(lineattr) *, MPT_INTERFACE(convertable) *);
extern int mpt_lattr_symbol(MPT_STRUCT(lineattr) *, MPT_INTERFACE(convertable) *);
extern int mpt_lattr_size  (MPT_STRUCT(lineattr) *, MPT_INTERFACE(convertable) *);
extern int mpt_lattr_set(MPT_STRUCT(lineattr) *, int , int , int , int);

/* change string data */
extern int mpt_string_pset(char **, MPT_INTERFACE(convertable) *);
extern int mpt_string_set(char **, const char *, int __MPT_DEFPAR(-1));

/* create pline representation for axis ticks */
extern void mpt_ticks_linear(MPT_STRUCT(dpoint) *, size_t , double , double);
extern double mpt_tick_log10(int);

__MPT_EXTDECL_END

#ifdef __cplusplus
class parser;

template <> inline __MPT_CONST_TYPE int type_properties<color>::id(bool) {
	return TypeColor;
}
template <> inline const struct type_traits *type_properties<color>::traits() {
	return type_traits::get(id(true));
}

template <> inline __MPT_CONST_TYPE int type_properties<lineattr>::id(bool) {
	return TypeLineAttr;
}
template <> inline const struct type_traits *type_properties<lineattr>::traits() {
	return type_traits::get(id(true));
}

template <> inline __MPT_CONST_TYPE int type_properties<line>::id(bool) {
	return TypeLine;
}
template <> inline const struct type_traits *type_properties<line *>::traits() {
	return type_traits::get(id(true));
}

template <> inline __MPT_CONST_TYPE int type_properties<axis *>::id(bool) {
	return TypeAxisPtr;
}
template <> inline const struct type_traits *type_properties<axis *>::traits() {
	return type_traits::get(id(true));
}

template <> inline __MPT_CONST_TYPE int type_properties<world *>::id(bool) {
	return TypeWorldPtr;
}
template <> inline const struct type_traits *type_properties<world *>::traits() {
	return type_traits::get(id(true));
}

template <> inline __MPT_CONST_TYPE int type_properties<graph *>::id(bool) {
	return TypeGraphPtr;
}
template <> inline const struct type_traits *type_properties<graph *>::traits() {
	return type_traits::get(id(true));
}

template <> inline __MPT_CONST_TYPE int type_properties<text *>::id(bool) {
	return TypeTextPtr;
}
template <> inline const struct type_traits *type_properties<text *>::traits() {
	return type_traits::get(id(true));
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



/*! Group implementation using metatype item array */
class item_group : public metatype, public group
{
public:
	virtual ~item_group();
	
	int convert(int, void *) __MPT_OVERRIDE;
	
	void unref() __MPT_OVERRIDE;
	item_group *clone() const __MPT_OVERRIDE;
	
	int each(item_handler_t *, void *) const __MPT_OVERRIDE;
	int append(const identifier *, metatype *) __MPT_OVERRIDE;
	size_t clear(const metatype * = 0) __MPT_OVERRIDE;
	int bind(const relation *, logger * = logger::default_instance()) __MPT_OVERRIDE;
	
	inline span<const item<metatype> > items() const
	{
		return _items.elements();
	}
protected:
	item_array<metatype> _items;
};

/*! Represent elements in layout file */
class layout : public item_group, public object
{
public:
	class line;
	class text;
	
	class graph;
	
	layout();
	~layout() __MPT_OVERRIDE;
	
	int convert(int , void *) __MPT_OVERRIDE;
	
	int property(struct property *) const __MPT_OVERRIDE;
	int set_property(const char *, convertable *) __MPT_OVERRIDE;
	
	int bind(const relation *, logger * = logger::default_instance()) __MPT_OVERRIDE;
	
	virtual bool load(logger * = logger::default_instance());
	virtual bool open(const char *);
	virtual bool reset();
	
	inline span<const item<graph> > graphs() const
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
	
	static const char *file_format();
	
	fpoint minimal_scale() const;
protected:
	item_array<graph> _graphs;
	parser *_parse;
	char *_alias;
	char *_font;
};

class layout::line : public metatype, public object, public ::mpt::line
{
public:
	line(const ::mpt::line *from = 0);
	virtual ~line();
	
	int convert(int, void *) __MPT_OVERRIDE;
	
	void unref() __MPT_OVERRIDE;
	line *clone() const __MPT_OVERRIDE;
	
	int property(struct property *) const __MPT_OVERRIDE;
	int set_property(const char *, convertable * = 0) __MPT_OVERRIDE;
};

class layout::text : public metatype, public object, public ::mpt::text
{
public:
	text(const ::mpt::text *from = 0);
	virtual ~text();
	
	int convert(int, void *) __MPT_OVERRIDE;
	
	void unref() __MPT_OVERRIDE;
	text *clone() const __MPT_OVERRIDE;
	
	int property(struct property *) const __MPT_OVERRIDE;
	int set_property(const char *, convertable *) __MPT_OVERRIDE;
};

/*! Container and binding for data to axes */
class layout::graph : public item_group, public object, public ::mpt::graph
{
public:
	class axis;
	class world;
	class transform3;
	class data
	{
	public:
		data(class world *w = 0);
		
		/* required for reference */
		virtual void unref() = 0;
		virtual uintptr_t addref() = 0;
		
		reference<class world> world;
		reference<class cycle> cycle;
	protected:
		virtual ~data()
		{ }
	};
	
	graph(const ::mpt::graph * = 0);
	~graph() __MPT_OVERRIDE;
	
	int convert(int, void *) __MPT_OVERRIDE;
	
	graph *clone() const __MPT_OVERRIDE;
	
	int property(struct property *) const __MPT_OVERRIDE;
	int set_property(const char *, convertable *) __MPT_OVERRIDE;
	
	int bind(const relation *, logger * = logger::default_instance()) __MPT_OVERRIDE;
	
	virtual item<axis> *add_axis(axis * = 0, const char * = 0, int = -1);
	inline span<const item<axis> > axes() const
	{
		return _axes.elements();
	}
	virtual item<data> *add_world(world * = 0, const char * = 0, int = -1);
	inline span<const item<data> > worlds() const
	{
		return _worlds.elements();
	}
	virtual bool set_cycle(int , const reference<class cycle> &) const;
	virtual const reference<class cycle> *cycle(int) const;
	
	const ::mpt::transform &transform();
	
	const struct value_apply *transform_part(int = -1) const;
	int transform_flags(int = -1) const;
	
	bool update_transform(int dim = -1);
	
protected:
	reference<class transform3> _gtr;
	item_array<axis> _axes;
	item_array<data> _worlds;
};

/*! Transformation parameters/interface for (up to) 3 dimensions */
class layout::graph::transform3 : public reference< ::mpt::transform>::type
{
public:
	struct data : public value_apply
	{
		data(int = -1);
		
		struct range limit;
	};
	
	transform3();
	~transform3() __MPT_OVERRIDE;
	
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
	
	int convert(int, void *) __MPT_OVERRIDE;
	
	void unref() __MPT_OVERRIDE;
	axis *clone() const __MPT_OVERRIDE;
	
	int property(struct property *) const __MPT_OVERRIDE;
	int set_property(const char *, convertable *) __MPT_OVERRIDE;
};

class layout::graph::world : public metatype, public object, public ::mpt::world
{
public:
	world(int = 0);
	world(const ::mpt::world *);
	virtual ~world();
	
	int convert(int, void *) __MPT_OVERRIDE;
	
	void unref() __MPT_OVERRIDE;
	world *clone() const __MPT_OVERRIDE;
	
	int property(struct property *) const __MPT_OVERRIDE;
	int set_property(const char *, convertable *) __MPT_OVERRIDE;
};

#endif

__MPT_NAMESPACE_END

#endif /* _MPT_LAYOUT_H */

