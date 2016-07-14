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
# include <limits>
#else
# include "core.h"
#endif

struct iovec;

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(array);
MPT_INTERFACE(metatype);

enum MPT_ENUM(AxisFlag) {
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

enum MPT_ENUM(TextFlag) {
	MPT_ENUM(TextItalic)  = 0x1,
	MPT_ENUM(TextBold)    = 0x2
};

/* primitive type point/transformation structure */
#ifdef __cplusplus
template<typename T>
struct point
{
    point(T _x = 0, T _y = 0) : x(_x), y(_y)
    { }
    point(const point &from) : x(from.x), y(from.y)
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
	axis(AxisFlag type = AxisStyleGen);
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

/* line part metadate */
MPT_STRUCT(linepart)
{
#ifdef __cplusplus
	linepart(int len = 0, int _raw = 0);
	
	linepart *join(const linepart &);
	
	float cut() const;
	float trim() const;
	
	int cutRaw() const;
	int trimRaw() const;
	
	bool setTrim(float );
	bool setCut(float );
	bool setRaw(int, int );
#endif
	uint16_t raw,   /* raw points in line part */
	         usr,   /* transformed points in line part */
	        _cut,
	        _trim;  /* remove fraction from line start/end */
};

MPT_STRUCT(transform)
{
#ifdef __cplusplus
	transform(enum AxisFlag = AxisStyleGen);
	
	int fromAxis(const axis &, int type = -1);
#endif
	MPT_STRUCT(dpoint) scale,  /* scale factor */
	                   move;   /* move start position */
	double             min,    /* minimal value */
	                   max;    /* maximal value */
};

/* part of MPT world */
#ifdef __cplusplus
MPT_INTERFACE(polyline) : public unrefable
{
    public:
	virtual void *raw(int dim, size_t len, size_t off = 0) = 0;
	virtual ssize_t truncate(int dim = -1, ssize_t = -1) = 0;
	virtual const char *format() const;
    protected:
	inline ~polyline() { }
};
#else
MPT_INTERFACE(polyline);
MPT_INTERFACE_VPTR(polyline) {
	MPT_INTERFACE_VPTR(unrefable) ref;
	void *(*raw)(MPT_INTERFACE(polyline) *, int , size_t , size_t);
	ssize_t (*truncate)(MPT_INTERFACE(polyline) *, int , ssize_t);
	const char *(*format)(const MPT_INTERFACE(polyline) *);
}; MPT_INTERFACE(polyline) {
	const MPT_INTERFACE_VPTR(polyline) *_vptr;
};
#endif

/* part of MPT world */
#ifdef __cplusplus
MPT_INTERFACE(cycle) : public unrefable
{
    public:
	enum { Type = TypeCycle };
	
	virtual polyline *current() const = 0;
	virtual polyline *advance() = 0;
	virtual polyline *append() = 0;
	
	virtual polyline *part(int pos = 0) const = 0;
	virtual int size() const = 0;
    protected:
	inline ~cycle() { }
};
#else
MPT_INTERFACE(cycle);
MPT_INTERFACE_VPTR(cycle) {
	MPT_INTERFACE_VPTR(unrefable) ref;
	
	MPT_INTERFACE(polyline) *(*current)(const MPT_INTERFACE(cycle) *);
	MPT_INTERFACE(polyline) *(*advance)(MPT_INTERFACE(cycle) *);
	MPT_INTERFACE(polyline) *(*append)(MPT_INTERFACE(cycle) *);
	
	MPT_INTERFACE(polyline) *(*part)(const MPT_INTERFACE(cycle) *, int pos);
	int (*size)(const MPT_INTERFACE(cycle) *);
}; MPT_INTERFACE(cycle) {
	const MPT_INTERFACE_VPTR(cycle) *_vptr;
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

extern void mpt_trans_init(MPT_STRUCT(transform) *, enum MPT_ENUM(AxisFlag) __MPT_DEFPAR(AxisStyleGen));

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
extern int mpt_lattr_set(MPT_STRUCT(lineattr) *, int , int , int , int );
extern int mpt_color_set(MPT_STRUCT(color) *, int , int , int );
extern int mpt_color_setalpha(MPT_STRUCT(color) *, int );

/* basic double data cycle part creation/destruction */
extern MPT_INTERFACE(polyline) *mpt_pline_create(void);
/* create cycle interface with basic polylines */
extern MPT_INTERFACE(cycle) *mpt_cycle_create(void);

/* set all array data to same/maximum size */
extern ssize_t mpt_pline_truncate(MPT_STRUCT(array) *, size_t , ssize_t __MPT_DEFPAR(-1));

/* create linear part in range */
extern void mpt_linepart_linear(MPT_STRUCT(linepart) *, const double *, size_t , const MPT_STRUCT(dpoint) *);
/* length of pline user points */
extern size_t mpt_linepart_ulength(const MPT_STRUCT(linepart) *, size_t );
extern size_t mpt_linepart_rlength(const MPT_STRUCT(linepart) *, size_t );
/* encode/decode linepart trim and cut values */
extern int mpt_linepart_code(double val);
extern double mpt_linepart_real(int val);
/* join parts if allowed by size and no userdata in second */
extern MPT_STRUCT(linepart) *mpt_linepart_join(MPT_STRUCT(linepart) *to, const MPT_STRUCT(linepart) *post);

/* apply raw to user data using scale parameter and (transform function) */
extern void mpt_apply_linear(MPT_STRUCT(dpoint) *, const MPT_STRUCT(linepart) *, const double *, const MPT_STRUCT(dpoint) *);

/* create pline representation for axis ticks */
extern void mpt_ticks_linear(MPT_STRUCT(dpoint) *, size_t , double , double);
extern double mpt_tick_log10(int);

#ifdef _MPT_MESSAGE_H
/* data mapping operations */
extern int mpt_mapping_add(MPT_STRUCT(array) *, const MPT_STRUCT(mapping) *);
extern int mpt_mapping_del(const MPT_STRUCT(array) *, const MPT_STRUCT(msgbind) *, const MPT_STRUCT(msgdest) * __MPT_DEFPAR(0), int __MPT_DEFPAR(0));
extern int mpt_mapping_cmp(const MPT_STRUCT(mapping) *, const MPT_STRUCT(msgbind) *, int __MPT_DEFPAR(0));
#endif

__MPT_EXTDECL_END

#ifdef __cplusplus
class Parse;
struct parseflg;
struct path;

inline linepart::linepart(int len, int raw) : raw(raw >= 0 ? raw : len), usr(len), _cut(0), _trim(0)
{ }
inline int linepart::cutRaw() const
{ return _cut; }
inline int linepart::trimRaw() const
{ return _trim; }

inline bool linepart::setRaw(int c, int t)
{
    if (c > std::numeric_limits<__decltype(_cut)>::max()
        || c < std::numeric_limits<__decltype(_cut)>::min()
        || t > std::numeric_limits<__decltype(_trim)>::max()
        || t < std::numeric_limits<__decltype(_trim)>::min()) {
        return false;
    }
    _cut = c; _trim = t;
    return true;
}

class Transform
{
public:
    virtual int dimensions() const = 0;
    
    virtual linepart part(int dim, const double *val, int len) const;
    virtual bool apply(int dim, const linepart &, dpoint *dest, const double *from) const;
    virtual dpoint zero() const;
protected:
    inline ~Transform() { }
};

extern int applyLineData(point<double> *dest, const linepart *lp, int plen, Transform &tr, polyline &part);

class Transform3 : public Transform
{
public:
    Transform3();
    
    inline virtual ~Transform3()
    { }
    
    int dimensions() const;
    
    linepart part(int dim, const double *val, int len) const;
    bool apply(int dim, const linepart &pt, dpoint *dest, const double *from) const;
    dpoint zero() const;
    
    transform tx, ty, tz; /* dimension transformations */
    uint8_t fx, fy, fz;   /* transformation options */
    uint8_t cut;
};

class Polyline : public polyline
{
public:
#if __cplusplus < 201103L
    typedef dpoint Point;
#else
    using Point = point<double>;
#endif
    class Parts : public ::mpt::array
    {
    public:
        inline Slice<const linepart> data() const
        { return Slice<const linepart>((linepart *) base(), length()/sizeof(linepart)); }
        
        bool create(const Transform &, polyline &);
        size_t userLength();
        size_t rawLength();
    };
    class Points : public ::mpt::array
    {
    public:
        Slice<const Point> data() const;
        Point *resize(size_t);
        inline size_t length() const
        { return array::length() / sizeof(Point); }
    };
    Polyline(int dim = -1);
    virtual ~Polyline();
    
    void unref();
    void *raw(int dim, size_t len, size_t off = 0);
    ssize_t truncate(int dim = -1, ssize_t pos = -1);
    const char *format() const;
    
    virtual Slice<Point> values(int part = -1) const;
    virtual Slice<const linepart> vis() const;
    virtual void transform(const Transform &);
    
    bool setValues(int dim, size_t len, const double *val, int ld = 1, size_t offset = 0);
    void setModified(int dim = -1);
    bool modified(int dim = -1) const;
    
protected:
    Array<array> _d;
    
    char *_rawType;
    
    uint32_t _modified;
    uint16_t _rawDim;
    uint8_t  _rawSize;
    uint8_t  _dataSize;
    
    array *rawData(int dim) const;
    Points *userData() const;
};

class Cycle : public cycle
{
public:
    Cycle(int dim = -1);
    
    enum Flags {
        AutoGrow = 0x1
    };
    void unref();
    virtual uintptr_t addref();
    
    inline const Reference<Polyline> *begin() const
    { return _part.begin(); }
    
    inline const Reference<Polyline> *end() const
    { return _part.end(); }
    
    Polyline *current() const;
    Polyline *advance();
    Polyline *append();
    
    Polyline *part(int pos) const;
    int size() const;
    
    virtual bool setSize(int cycles);
    virtual bool updateTransform(const Transform &, bool = false);
    
protected:
    virtual ~Cycle();
    RefArray<Polyline> _part;
    uint16_t _act;
    uint8_t  _dim;
    uint8_t  _flags;
};

class Line : public object, public metatype, public line
{
public:
    enum { Type = line::Type };
    
    Line(const line *from = 0);
    virtual ~Line();
    
    void unref();
    int property(struct property *) const;
    int setProperty(const char *, metatype * = 0);
    
    int assign(const struct value *);
    int conv(int, void *);
    metatype *clone() const;
};

class Text : public object, public metatype, public text
{
public:
    enum { Type = text::Type };
    
    Text(const text *from = 0);
    virtual ~Text();
    
    void unref();
    int property(struct property *) const;
    int setProperty(const char *, metatype *);
    
    int assign(const struct value *);
    int conv(int, void *);
    metatype *clone() const;
};

class Axis : public object, public metatype, public axis
{
public:
    enum { Type = axis::Type };
    
    Axis(const axis *from = 0);
    Axis(AxisFlag type);
    virtual ~Axis();
    
    void unref();
    int property(struct property *) const;
    int setProperty(const char *, metatype *);
    
    int assign(const struct value *);
    int conv(int, void *);
    metatype *clone() const;
};

class World : public object, public metatype, public world
{
public:
    enum { Type = world::Type };
    
    World(int = 0);
    World(const world *);
    virtual ~World();
    
    void unref();
    int property(struct property *) const;
    int setProperty(const char *, metatype *);
    
    int assign(const struct value *);
    int conv(int, void *);
    metatype *clone() const;
};

/*! Group implementation using reference array */
class Collection : public Group
{
public:
    virtual ~Collection();
    
    const Item<metatype> *item(size_t) const;
    Item<metatype> *append(metatype *);
    size_t clear(const metatype * = 0);
    bool bind(const Relation &, logger * = logger::defaultInstance());
    
protected:
    ItemArray<metatype> _items;
};

class Graph : public Collection, public metatype, public Transform3, public graph
{
public:
    class Data : public Item<World>
    {
    public:
        Data(World *w = 0);
        
        inline void unref()
        { delete this; }
        
        Reference<Cycle> cycle;
    };
    enum { Type = graph::Type };
    
    Graph(const graph * = 0);
    virtual ~Graph();
    
    void unref();
    int assign(const struct value *);
    int conv(int, void *);
    metatype *clone() const;
    
    int property(struct property *) const;
    int setProperty(const char *, metatype *);
    
    bool bind(const Relation &from, logger * = logger::defaultInstance());
    
    virtual Item<Axis> *addAxis(Axis * = 0, const char * = 0, int = -1);
    const Item<Axis> &axis(int pos) const;
    inline int axisCount() const { return _axes.length(); }
    
    virtual Data *addWorld(World * = 0, const char * = 0, int = -1);
    const Data &world(int pos) const;
    inline int worldCount() const { return _worlds.length(); }
    
    virtual bool setCycle(int pos, const Reference<Cycle> &) const;
    virtual const Reference<Cycle> &cycle(int pos) const;
    
    const Transform &transform();
    bool updateTransform(int dim = -1);
    
protected:
    ItemArray<Axis> _axes;
    RefArray<Data>  _worlds;
};

class Layout : public Collection, public metatype
{
public:
    enum { Type = Collection::Type };
    
    Layout();
    virtual ~Layout();
    
    void unref();
    int assign(const struct value *);
    int conv(int, void *);
    metatype *clone() const;
    
    int property(struct property *pr) const;
    int setProperty(const char *pr, metatype *src);
    
    bool bind(const Relation &, logger * = logger::defaultInstance());
    
    virtual bool update(metatype *);
    virtual bool load(logger * = logger::defaultInstance());
    virtual bool open(const char *);
    virtual void reset();
    
    const Item<Graph> &graph(int pos) const;
    inline int graphCount() const { return _graphs.length(); }
    
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
#endif

__MPT_NAMESPACE_END

#endif /* _MPT_LAYOUT_H */

