/*!
 * MPT plotting library
 *  value creation and processing
 */

#ifndef _MPT_VALUES_H
#define _MPT_VALUES_H  @INTERFACE_VERSION@

#include "array.h"

__MPT_NAMESPACE_BEGIN

enum MPT_ENUM(ValueDescription)
{
	MPT_ENUM(ValueFormatLinear) = 1,
	MPT_ENUM(ValueFormatBoundaries),
	MPT_ENUM(ValueFormatText),
	MPT_ENUM(ValueFormatPolynom),
	MPT_ENUM(ValueFormatFile)
};

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

MPT_STRUCT(rawdata_stage)
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
#else
# define MPT_RAWDATA_STAGE_INIT { MPT_ARRAY_INIT, 0 }
#endif
	_MPT_ARRAY_TYPE(typed_array) _d;
	uint8_t _maxDimensions;
};

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

/* line part properties */
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

/* binding to layout mapping */
MPT_STRUCT(mapping)
#ifdef _MPT_MESSAGE_H
{
#ifdef __cplusplus
	inline mapping(const msgbind &m = msgbind(0), const msgdest &d = msgdest(), int c = 0) :
		src(m), client(c), dest(d)
	{ }
	inline bool valid() const
	{ return src.state != 0; }
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

/* create profile of specific type */
extern int mpt_values_linear(int , double *, int , double , double);
extern int mpt_values_bound (int , double *, int , double , double , double);
extern int mpt_values_string(const char *, int , double *, int);
extern int mpt_values_poly  (const char *, int , double *, int , const double *);

/* prepare values on buffer offset */
extern double *mpt_values_prepare(_MPT_ARRAY_TYPE(double) *, int);

/* select/set solver profile type */
extern int mpt_valtype_select(const char **);
extern int mpt_valtype_init(int , const char *, int , double *, int , const double *);

/* create iterator (descr. includes type info) */
extern MPT_INTERFACE(iterator) *mpt_iterator_create(const char *);
/* create specific iterator */
extern MPT_INTERFACE(iterator) *_mpt_iterator_range (MPT_STRUCT(value));
extern MPT_INTERFACE(iterator) *_mpt_iterator_linear(MPT_STRUCT(value));
extern MPT_INTERFACE(iterator) *_mpt_iterator_values(MPT_STRUCT(value));
extern MPT_INTERFACE(iterator) *_mpt_iterator_factor(MPT_STRUCT(value));

#if defined(_STDIO_H) || defined(_STDIO_H_)
/* set solver matrix via file */
extern int mpt_values_file(FILE *, int , int , double *);
#endif


/* multi dimension data operations */
extern MPT_STRUCT(typed_array) *mpt_stage_data(MPT_STRUCT(rawdata_stage) *, unsigned);
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

/* set point data */
extern int mpt_fpoint_set(MPT_STRUCT(fpoint) *, const MPT_INTERFACE(metatype) *, const MPT_STRUCT(fpoint) *__MPT_DEFPAR(0), const MPT_STRUCT(range) *__MPT_DEFPAR(0));

/* append values described by string */
extern double *mpt_values_generate(_MPT_ARRAY_TYPE(double) *, int , const char *);


#ifdef _MPT_MESSAGE_H
/* data mapping operations */
extern int mpt_mapping_add(_MPT_ARRAY_TYPE(mapping) *, const MPT_STRUCT(mapping) *);
extern int mpt_mapping_del(const _MPT_ARRAY_TYPE(mapping) *, const MPT_STRUCT(msgbind) *, const MPT_STRUCT(msgdest) * __MPT_DEFPAR(0), int __MPT_DEFPAR(0));
extern int mpt_mapping_cmp(const MPT_STRUCT(mapping) *, const MPT_STRUCT(msgbind) *, int __MPT_DEFPAR(0));
#endif

__MPT_EXTDECL_END

#ifdef __cplusplus

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
    Cycle();
    
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
};template <typename T, typename S>
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

#endif /* _MPT_VALUES_H */
