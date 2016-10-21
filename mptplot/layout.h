/*!
 * MPT core library
 *  layout data structures
 */

#ifndef _MPT_LAYOUT_H
#define _MPT_LAYOUT_H  @INTERFACE_VERSION@

#ifdef __cplusplus
# include "meta.h"
# include "array.h"
# include "object.h"
#else
# include "core.h"
#endif

struct iovec;

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(array);
MPT_STRUCT(typed_array);
MPT_INTERFACE(metatype);

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

/* forward declaration */
MPT_STRUCT(rawdata);

/* primitive type point/transformation structure */
#ifdef __cplusplus
template<typename T>
struct point
{
    inline point(T _x = 0, T _y = 0) : x(_x), y(_y)
    { }
    inline point(const point &from) : x(from.x), y(from.y)
    { }
    inline bool operator ==(const point &p) const
    { return x == p.x && y == p.y; }

    inline point & operator +=(const point &p) const
    { return x += p.x; y += p.y; return *this; }
    inline point & operator -=(const point &p) const
    { return x -= p.x; y -= p.y; return *this; }

    T x, y;
};
struct dpoint : public point<double>{ dpoint(double _x = 0, double _y = 0) : point(_x, _y) {} };
struct fpoint : public point<float> { fpoint(float  _x = 0, float  _y = 0) : point(_x, _y) {} };
#else
MPT_STRUCT(dpoint) { double x, y; };
MPT_STRUCT(fpoint) { float x, y; };
#endif

/* value range */
MPT_STRUCT(range)
{
#ifdef __cplusplus
	range();
	inline range(double _min, double _max) : min(_min), max(_max)
	{ }
#endif
	double min, max;
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

/* simple line/rectangle */
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

/* axis data */
MPT_STRUCT(axis)
{
#ifdef __cplusplus
	axis(AxisFlags type = AxisStyleGen);
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

/* format of data */
MPT_STRUCT(world)
{
#ifdef __cplusplus
	world();
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
	text(const text *t = 0);
	text & operator= (const text & tx);
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

/* line part metadate */
MPT_STRUCT(linepart)
{
#ifdef __cplusplus
	linepart(int = 0, int = -1);
	
	linepart *join(linepart);
	
	float cut() const;
	float trim() const;
	
	bool setTrim(float);
	bool setCut(float);
	
	class array : public Array<linepart>
	{
	public:
		bool apply(const Transform &, int , Slice<const double>);
		bool set(size_t);
		size_t userLength();
		size_t rawLength();
	};
#endif
	uint16_t raw,   /* raw points in line part */
	         usr,   /* transformed points in line part */
	        _cut,
	        _trim;  /* remove fraction from line start/end */
};

MPT_STRUCT(rawdata_stage)
#ifdef _MPT_ARRAY_H
{
#ifdef __cplusplus
    public:
	int modify(unsigned , int , const void *, size_t , size_t);
	int clearModified(int = -1);
	
	inline const typed_array *values(int dim) const
	{ return dim < 0 ? 0 : _d.get(dim); }
	inline Slice<const typed_array> values() const
	{ return _d.slice(); }
	inline int dimensions() const
	{ return _d.length(); }
    protected:
#endif
	_MPT_ARRAY_TYPE(typed_array) _d;
}
#endif
;

/* part of MPT world */
#ifdef __cplusplus
MPT_INTERFACE(rawdata) : public unrefable
{
    public:
	enum { Type = TypeRawData };
	
	virtual int modify(unsigned , int , const void *, size_t , size_t, int = -1) = 0;
	virtual int advance() = 0;
	
	virtual int values(unsigned , struct iovec * = 0, int = -1) const = 0;
	virtual int dimensions(int = -1) const = 0;
	virtual int stages() const = 0;
    protected:
	inline ~rawdata() { }
};
#else
MPT_INTERFACE(rawdata);
MPT_INTERFACE_VPTR(rawdata) {
	MPT_INTERFACE_VPTR(unrefable) ref;
	
	int (*modify)(MPT_INTERFACE(rawdata) *, unsigned , int , const void *, size_t , size_t , int);
	int (*advance)(MPT_INTERFACE(rawdata) *);
	
	int (*values)(const MPT_INTERFACE(rawdata) *, unsigned , struct iovec *, int);
	int (*dimensions)(const MPT_INTERFACE(rawdata) *, int);
	int (*stages)(const MPT_INTERFACE(rawdata) *);
}; MPT_INTERFACE(rawdata) {
	const MPT_INTERFACE_VPTR(rawdata) *_vptr;
};
#endif

/* binding to layout mapping */
MPT_STRUCT(mapping)
#ifdef _MPT_MESSAGE_H
{
#ifdef __cplusplus
	inline mapping(const msgbind &m = msgbind(0), const msgdest &d = msgdest(), int c = 0) :
		src(m), client(c), dest(d)
	{ }
	inline bool valid() const
	{ return src.type != 0; }
#else
# define MPT_MAPPING_INIT { MPT_MSGBIND_INIT, 0, MPT_MSGDEST_INIT }
#endif
	MPT_STRUCT(msgbind) src;
	uint16_t            client;
	MPT_STRUCT(msgdest) dest;
}
#endif
;

__MPT_EXTDECL_BEGIN

extern int mpt_color_parse(MPT_STRUCT(color) *color, const char *txt);
extern int mpt_color_html (MPT_STRUCT(color) *color, const char *txt);

extern void mpt_trans_init(MPT_STRUCT(transform) *, enum MPT_ENUM(AxisFlags) __MPT_DEFPAR(AxisStyleGen));

extern void mpt_line_init(MPT_STRUCT(line) *);
extern int  mpt_line_get (const MPT_STRUCT(line) *, MPT_STRUCT(property) *);
extern int  mpt_line_set (MPT_STRUCT(line) *, const char *, MPT_INTERFACE(metatype) *);

extern void mpt_graph_init(MPT_STRUCT(graph) *, const MPT_STRUCT(graph) *__MPT_DEFPAR(0));
extern void mpt_graph_fini(MPT_STRUCT(graph) *);
extern int  mpt_graph_get (const MPT_STRUCT(graph) *, MPT_STRUCT(property) *);
extern int  mpt_graph_set (MPT_STRUCT(graph) *, const char *, MPT_INTERFACE(metatype) *);

extern void mpt_axis_init(MPT_STRUCT(axis) *, const MPT_STRUCT(axis) *__MPT_DEFPAR(0));
extern void mpt_axis_fini(MPT_STRUCT(axis) *);
extern int  mpt_axis_get (const MPT_STRUCT(axis) *, MPT_STRUCT(property) *);
extern int  mpt_axis_set (MPT_STRUCT(axis) *, const char *, MPT_INTERFACE(metatype) *);

extern void mpt_world_init(MPT_STRUCT(world) *, const MPT_STRUCT(world) *__MPT_DEFPAR(0));
extern void mpt_world_fini(MPT_STRUCT(world) *);
extern int  mpt_world_get (const MPT_STRUCT(world) *, MPT_STRUCT(property) *);
extern int  mpt_world_set (MPT_STRUCT(world) *, const char *, MPT_INTERFACE(metatype) *);

extern void mpt_text_init(MPT_STRUCT(text) *, const MPT_STRUCT(text) *__MPT_DEFPAR(0));
extern void mpt_text_fini(MPT_STRUCT(text) *);
extern int  mpt_text_get (const MPT_STRUCT(text) *, MPT_STRUCT(property) *);
extern int  mpt_text_set (MPT_STRUCT(text) *, const char *, MPT_INTERFACE(metatype) *);

/* set axis type and lenth */
extern void mpt_axis_setx(MPT_STRUCT(axis) *, double );
extern void mpt_axis_sety(MPT_STRUCT(axis) *, double );
extern void mpt_axis_setz(MPT_STRUCT(axis) *, double );

/* general value setter */
extern int mpt_lattr_style (MPT_STRUCT(lineattr) *, MPT_INTERFACE(metatype) *);
extern int mpt_lattr_width (MPT_STRUCT(lineattr) *, MPT_INTERFACE(metatype) *);
extern int mpt_lattr_symbol(MPT_STRUCT(lineattr) *, MPT_INTERFACE(metatype) *);
extern int mpt_lattr_size  (MPT_STRUCT(lineattr) *, MPT_INTERFACE(metatype) *);

extern int mpt_color_pset(MPT_STRUCT(color) *, MPT_INTERFACE(metatype) *);
extern int mpt_string_pset(char **, MPT_INTERFACE(metatype) *);
extern int mpt_string_set(char **, const char *, int __MPT_DEFPAR(-1));

/* set line/color attributes */
extern int mpt_lattr_set(MPT_STRUCT(lineattr) *, int , int , int , int);
extern int mpt_color_set(MPT_STRUCT(color) *, int , int , int);
extern int mpt_color_setalpha(MPT_STRUCT(color) *, int);

/* multi dimension data operations */
extern MPT_STRUCT(typed_array) *mpt_stage_data(MPT_STRUCT(rawdata_stage) *, unsigned , int __MPT_DEFPAR(-1));
extern void mpt_stage_fini(MPT_STRUCT(rawdata_stage) *);
/* set dimensions to defined size */
extern ssize_t mpt_stage_truncate(MPT_STRUCT(rawdata_stage) *, size_t __MPT_DEFPAR(0));

/* create cycle interface with default data parts */
extern MPT_INTERFACE(rawdata) *mpt_rawdata_create(size_t);

/* create linear part in range */
extern void mpt_linepart_linear(MPT_STRUCT(linepart) *, const double *, size_t , const MPT_STRUCT(range) *);
/* encode/decode linepart trim and cut values */
extern int mpt_linepart_code(double val);
extern double mpt_linepart_real(int val);
/* join parts if allowed by size and no userdata in second */
extern MPT_STRUCT(linepart) *mpt_linepart_join(MPT_STRUCT(linepart) *, const MPT_STRUCT(linepart));

/* apply raw to user data using scale parameter and (transform function) */
extern void mpt_apply_linear(MPT_STRUCT(dpoint) *, const MPT_STRUCT(linepart) *, const double *, const MPT_STRUCT(dpoint) *);

/* create pline representation for axis ticks */
extern void mpt_ticks_linear(MPT_STRUCT(dpoint) *, size_t , double , double);
extern double mpt_tick_log10(int);

#ifdef _MPT_MESSAGE_H
/* data mapping operations */
extern int mpt_mapping_add(_MPT_ARRAY_TYPE(mapping) *, const MPT_STRUCT(mapping) *);
extern int mpt_mapping_del(const _MPT_ARRAY_TYPE(mapping) *, const MPT_STRUCT(msgbind) *, const MPT_STRUCT(msgdest) * __MPT_DEFPAR(0), int __MPT_DEFPAR(0));
extern int mpt_mapping_cmp(const MPT_STRUCT(mapping) *, const MPT_STRUCT(msgbind) *, int __MPT_DEFPAR(0));
#endif

__MPT_EXTDECL_END

#ifdef __cplusplus
class Parse;
struct parseflg;
struct path;

inline linepart::linepart(int usr, int raw) : raw(raw >= 0 ? raw : usr), usr(usr), _cut(0), _trim(0)
{ }

class Transform : public unrefable
{
public:
    virtual int dimensions() const = 0;
    
    virtual linepart part(unsigned , const double *, int) const;
    virtual bool apply(unsigned , const linepart &, point<double> *, const double *) const;
    virtual point<double> zero() const;
protected:
    inline ~Transform() { }
};

extern int applyLineData(point<double> *, const linepart *, int , Transform &, Slice<const typed_array>);

class Transform3 : public Transform
{
public:
    Transform3();
    
    inline virtual ~Transform3()
    { }
    inline void unref() __MPT_OVERRIDE
    { delete this; }
    
    int dimensions() const __MPT_OVERRIDE;
    
    linepart part(unsigned , const double *, int) const __MPT_OVERRIDE;
    bool apply(unsigned , const linepart &pt, point<double> *, const double *) const __MPT_OVERRIDE;
    point<double> zero() const __MPT_OVERRIDE;
    
    transform tx, ty, tz; /* dimension transformations */
    uint8_t fx, fy, fz;   /* transformation options */
    uint8_t cutoff;       /* limit data to range */
};

class Polyline
{
public:
    class Point : public point<double>
    {
    public:
        inline Point(const point<double> &from = point<double>(0, 0)) : point<double>(from)
        { }
        class array : public Array<Point>
        {
        public:
            inline Point *resize(size_t len)
            { return reinterpret_cast<Point *>(_d.set(len * sizeof(Point))); }
        };
    };
    class Part
    {
    public:
        Part(const linepart lp, const Point *pts) : _part(lp), _pts(pts)
        { }
        Slice<const Point> line() const;
        Slice<const Point> points() const;
    protected:
        linepart _part;
        const Point *_pts;
    };
    class iterator
    {
    public:
        iterator(Slice<const linepart> l, const Point *p) : _parts(l), _points(p)
        { }
        Part operator *() const;
        iterator & operator++ ();
        
        bool operator== (const iterator &it) const
        { return _points == it._points; }
        inline bool operator!= (const iterator &it) const
        { return !operator==(it); }
    protected:
        Slice<const linepart> _parts;
        const Point *_points;
    };
    bool set(const Transform &, const rawdata &, int = -1);
    bool set(const Transform &, Slice<const typed_array>);

    iterator begin() const;
    iterator end() const;
    
    inline void clear()
    { _vis.set(0); _values.resize(0); }
    
    Slice<const linepart> parts() const
    { return _vis.slice(); }
    Slice<const Point> points() const
    { return _values.slice(); }
protected:
    linepart::array _vis;
    Point::array _values;
};

class Cycle : public rawdata
{
public:
    enum Flags {
        LimitStages = 1
    };
    inline Cycle() : _flags(0)
    { }
    
    class Stage : public rawdata_stage
    {
    public:
        inline const Polyline &values() const
        { return _values; }
        inline void invalidate()
        { _values.clear(); }
        bool transform(const Transform &);
    protected:
        Polyline _values;
    };
    /* add for extensions */
    virtual uintptr_t addref();
    
    /* basic raw data interface */
    void unref() __MPT_OVERRIDE;
    
    int modify(unsigned , int , const void *, size_t , size_t, int = -1) __MPT_OVERRIDE;
    int advance() __MPT_OVERRIDE;
    
    int values(unsigned , struct iovec * = 0, int = -1) const __MPT_OVERRIDE;
    int dimensions(int = -1) const __MPT_OVERRIDE;
    int stages() const __MPT_OVERRIDE;
    
    virtual void limitDimensions(uint8_t);
    virtual bool limitStages(size_t);
    
    inline Stage *begin()
    { return _stages.begin(); }
    inline Stage *end()
    { return _stages.end(); }
    
    Slice<const Stage> slice() const
    { return _stages.slice(); }
protected:
    virtual ~Cycle();
    Array<Stage> _stages;
    uint16_t _act;
    uint8_t _maxDimensions;
    uint8_t _flags;
};

class Line : public object, public line
{
public:
    enum { Type = line::Type };
    
    Line(const line *from = 0);
    virtual ~Line();
    
    void unref() __MPT_OVERRIDE;
    int property(struct property *) const __MPT_OVERRIDE;
    int setProperty(const char *, metatype * = 0) __MPT_OVERRIDE;
    
    virtual void *toType(int);
};

class Text : public object, public text
{
public:
    enum { Type = text::Type };
    
    Text(const text *from = 0);
    virtual ~Text();
    
    void unref() __MPT_OVERRIDE;
    int property(struct property *) const __MPT_OVERRIDE;
    int setProperty(const char *, metatype *) __MPT_OVERRIDE;
    
    virtual void *toType(int);
};

class Axis : public object, public axis
{
public:
    enum { Type = axis::Type };
    
    Axis(const axis *from = 0);
    Axis(AxisFlags type);
    virtual ~Axis();
    
    void unref() __MPT_OVERRIDE;
    int property(struct property *) const __MPT_OVERRIDE;
    int setProperty(const char *, metatype *) __MPT_OVERRIDE;
    
    virtual void *toType(int);
};

class World : public object, public world
{
public:
    enum { Type = world::Type };
    
    World(int = 0);
    World(const world *);
    virtual ~World();
    
    void unref() __MPT_OVERRIDE;
    int property(struct property *) const __MPT_OVERRIDE;
    int setProperty(const char *, metatype *) __MPT_OVERRIDE;
    
    virtual void *toType(int);
};

/*! Group implementation using reference array */
class Collection : public Group
{
public:
    virtual ~Collection();
    
    const Item<object> *item(size_t) const __MPT_OVERRIDE;
    Item<object> *append(object *) __MPT_OVERRIDE;
    size_t clear(const unrefable * = 0) __MPT_OVERRIDE;
    bool bind(const Relation &, logger * = logger::defaultInstance()) __MPT_OVERRIDE;
protected:
    ItemArray<object> _items;
};

class Graph : public Collection, public Transform3, public graph
{
public:
    class Data : public unrefable
    {
    public:
        Data(World *w = 0);
        virtual ~Data()
        { }
        
        inline void unref()
        { delete this; }
        
        Reference<World> world;
        Reference<Cycle> cycle;
    };
    enum { Type = graph::Type };
    
    Graph(const graph * = 0);
    virtual ~Graph();
    
    void unref() __MPT_OVERRIDE;
    int property(struct property *) const __MPT_OVERRIDE;
    int setProperty(const char *, metatype *) __MPT_OVERRIDE;
    
    bool bind(const Relation &from, logger * = logger::defaultInstance()) __MPT_OVERRIDE;
    
    void *toType(int) __MPT_OVERRIDE;
    
    virtual Item<Axis> *addAxis(Axis * = 0, const char * = 0, int = -1);
    inline Slice<const Item<Axis> > axes() const
    { return _axes.slice(); }
    
    virtual Item<Data> *addWorld(World * = 0, const char * = 0, int = -1);
    inline Slice<const Item<Data> > worlds() const
    { return _worlds.slice(); }
    
    virtual bool setCycle(int pos, const Reference<Cycle> &) const;
    virtual const Reference<Cycle> *cycle(int pos) const;
    
    const Transform &transform();
    bool updateTransform(int dim = -1);
    
protected:
    ItemArray<Axis> _axes;
    ItemArray<Data> _worlds;
};

class Layout : public Collection
{
public:
    enum { Type = Collection::Type };
    
    Layout();
    virtual ~Layout();
    
    void unref() __MPT_OVERRIDE;
    int property(struct property *pr) const __MPT_OVERRIDE;
    int setProperty(const char *pr, metatype *src) __MPT_OVERRIDE;
    
    bool bind(const Relation &, logger * = logger::defaultInstance()) __MPT_OVERRIDE;
    
    virtual bool load(logger * = logger::defaultInstance());
    virtual bool open(const char *);
    virtual void reset();
    
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


template <typename T, typename S>
void apply(T *d, const linepart &pt, const S *src, const T &scale, S fcn(S))
{
    size_t j = 0, len = pt.usr;
    double sx(scale.x), sy(scale.y), part;

    if ((part = pt.cut())) {
        S s1, s2;
        ++j;
        s1 = fcn(src[0]);
        s2 = fcn(src[1]);
        part = s1 + part * (s2-s1);

        d->x += sx * part;
        d->y += sy * part;
    }
    if ((part = pt.trim())) {
        S s1, s2;
        --len;
        s1 = fcn(src[len-1]);
        s2 = fcn(src[len]);
        part = s2 + part * (s1-s2);

        d[len].x += sx * part;
        d[len].y += sy * part;
    }
    for ( ; j < len ; j++ ) {
        d[j].x += sx * fcn(src[j]);
        d[j].y += sy * fcn(src[j]);
    }
}
template <typename S>
inline void apply(point<S> *d, const linepart &pt, const S *src, const point<S> &scale, S fcn(S))
{
    return apply<point<S>, S>(d, pt, src, scale, fcn);
}
#endif

__MPT_NAMESPACE_END

#endif /* _MPT_LAYOUT_H */

