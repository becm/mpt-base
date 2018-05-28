/*!
 * MPT core library
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
	
	MPT_ENUM(AxisLg)        = 0x4,
	MPT_ENUM(TransformSwap) = 0x8
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
	{ return _title; }
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
	
	inline const char *alias() const { return _alias; }
	bool setAlias(const char *name, int len = -1);
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
	{ return _axes; }
	inline const char *worlds() const
	{ return _worlds; }
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
	
	bool setValue(const char *);
	inline const char *value() const
	{ return _value; }
	
	bool setFont(const char *);
	inline const char *font() const
	{ return _font; }
	
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

MPT_STRUCT(transform)
{
#ifdef __cplusplus
	transform(enum AxisFlags = AxisStyleGen);
	
	int fromAxis(const axis &, int type = -1);
#endif
	MPT_STRUCT(dpoint) scale,  /* scale factor */
	                   move;   /* move start position */
	MPT_STRUCT(range)  limit;  /* value range */
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

/* initialize transform dimension */
extern void mpt_trans_init(MPT_STRUCT(transform) *, enum MPT_ENUM(AxisFlags) __MPT_DEFPAR(AxisStyleGen));

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

class Line : public metatype, public object, public line
{
public:
	Line(const line *from = 0);
	virtual ~Line();
	
	void unref() __MPT_OVERRIDE;
	int conv(int, void *) const __MPT_OVERRIDE;
	
	int property_get(struct property *) const __MPT_OVERRIDE;
	int property_set(const char *, const metatype * = 0) __MPT_OVERRIDE;
};

class Text : public metatype, public object, public text
{
public:
	Text(const text *from = 0);
	virtual ~Text();
	
	void unref() __MPT_OVERRIDE;
	int conv(int, void *) const __MPT_OVERRIDE;
	
	int property_get(struct property *) const __MPT_OVERRIDE;
	int property_set(const char *, const metatype *) __MPT_OVERRIDE;
};

class Axis : public metatype, public object, public axis
{
public:
	Axis(const axis *from = 0);
	Axis(AxisFlags type);
	virtual ~Axis();
	
	void unref() __MPT_OVERRIDE;
	int conv(int, void *) const __MPT_OVERRIDE;
	
	int property_get(struct property *) const __MPT_OVERRIDE;
	int property_set(const char *, const metatype *) __MPT_OVERRIDE;
};

class World : public metatype, public object, public world
{
public:
	World(int = 0);
	World(const world *);
	virtual ~World();
	
	void unref() __MPT_OVERRIDE;
	int conv(int, void *) const __MPT_OVERRIDE;
	
	int property_get(struct property *) const __MPT_OVERRIDE;
	int property_set(const char *, const metatype *) __MPT_OVERRIDE;
};

/*! Group implementation using reference array */
class Collection : public metatype, public Group
{
public:
	virtual ~Collection();
	
	void unref() __MPT_OVERRIDE;
	int conv(int, void *) const __MPT_OVERRIDE;
	Collection *clone() const __MPT_OVERRIDE;
	
	const Item<metatype> *item(size_t) const __MPT_OVERRIDE;
	Item<metatype> *append(metatype *) __MPT_OVERRIDE;
	size_t clear(const reference * = 0) __MPT_OVERRIDE;
	bool bind(const Relation &, logger * = logger::defaultInstance()) __MPT_OVERRIDE;
protected:
	ItemArray<metatype> _items;
};

/*! Transformation parameters/interface for (up to) 3 dimensions */
class Transform3 : public reference, public Transform
{
public:
	Transform3();
	
	inline virtual ~Transform3()
	{ }
	void unref() __MPT_OVERRIDE;
	
	int dimensions() const __MPT_OVERRIDE;
	
	linepart part(unsigned , const double *, int) const __MPT_OVERRIDE;
	bool apply(unsigned , const linepart &pt, point<double> *, const double *) const __MPT_OVERRIDE;
	point<double> zero() const __MPT_OVERRIDE;
	
	transform tx, ty, tz; /* dimension transformations */
	uint8_t fx, fy, fz;   /* transformation options */
	uint8_t cutoff;       /* limit data to range */
};

/*! Container and binding for data to axes */
class Graph : public Collection, public graph
{
public:
	class Data : public reference
	{
	public:
		Data(class World *w = 0);
		virtual ~Data()
		{ }
		
		void unref() __MPT_OVERRIDE;
		
		Reference<class World> world;
		Reference<class Cycle> cycle;
	};
	Graph(const graph * = 0);
	~Graph() __MPT_OVERRIDE;
	
	int conv(int, void *) const __MPT_OVERRIDE;
	
	int property_get(struct property *) const __MPT_OVERRIDE;
	int property_set(const char *, const metatype *) __MPT_OVERRIDE;
	
	bool bind(const Relation &from, logger * = logger::defaultInstance()) __MPT_OVERRIDE;
	
	virtual Item<class Axis> *addAxis(class Axis * = 0, const char * = 0, int = -1);
	inline Slice<const Item<class Axis> > axes() const
	{ return _axes.slice(); }
	
	virtual Item<class Data> *addWorld(class World * = 0, const char * = 0, int = -1);
	inline Slice<const Item<class Data> > worlds() const
	{ return _worlds.slice(); }
	
	virtual bool setCycle(int pos, const Reference<class Cycle> &) const;
	virtual const Reference<class Cycle> *cycle(int pos) const;
	
	const Transform &transform();
	
	const struct transform *getTransform(int = -1) const;
	int getFlags(int = -1) const;
	bool updateTransform(int dim = -1);
	
protected:
	Reference<Transform3> _gtr;
	ItemArray<Axis> _axes;
	ItemArray<Data> _worlds;
};

/*! Represent elements in layout file */
class Layout : public Collection
{
public:
	Layout();
	~Layout() __MPT_OVERRIDE;
	
	int property_get(struct property *pr) const __MPT_OVERRIDE;
	int property_set(const char *pr, const metatype *src) __MPT_OVERRIDE;
	
	bool bind(const Relation &, logger * = logger::defaultInstance()) __MPT_OVERRIDE;
	
	virtual bool load(logger * = logger::defaultInstance());
	virtual bool open(const char *);
	virtual bool reset();
	
	inline Slice<const Item<Graph> > graphs() const
	{ return _graphs.slice(); }
	
	virtual bool setAlias(const char *, int = -1);
	inline const char *alias() const
	{ return _alias; }
	
	virtual bool setFont(const char *, int = -1);
	inline const char *font() const
	{ return _font; }
	
	fpoint minScale() const;
	
protected:
	ItemArray<Graph> _graphs;
	Parse *_parse;
	char *_alias;
	char *_font;
};

#endif

__MPT_NAMESPACE_END

#endif /* _MPT_LAYOUT_H */

