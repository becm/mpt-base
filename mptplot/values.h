/*!
 * MPT plotting library
 *  value creation and processing
 */

#ifndef _MPT_VALUES_H
#define _MPT_VALUES_H  @INTERFACE_VERSION@

#include "array.h"

__MPT_NAMESPACE_BEGIN

MPT_INTERFACE(metatype);

/* primitive type point/transformation structure */
#ifdef __cplusplus
class Transform;

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

template<> int typeIdentifier<point<double> >();
template<> int typeIdentifier<point<float> >();
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

/*! information about containing data */
MPT_STRUCT(typed_array)
{
#ifdef __cplusplus
public:
	enum Flags {
		ValueChange = 1
	};
	inline typed_array() : _flags(0), _type(0), _esize(0)
	{ }
	bool setType(int);
	void *reserve(size_t , size_t);
	
	inline int flags() const
	{ return _flags; }
	void setModified(bool mod = true);
	
	inline size_t elementSize() const
	{ return _esize; }
	inline size_t elements() const
	{ return _esize ? _d.length() / _esize : 0; }
	inline const void *base() const
	{ return _d.base(); }
	
	inline int type() const
	{ return _type; }
protected:
#else
# define MPT_TYPED_ARRAY_INIT { MPT_ARRAY_INIT, 0, 0, 0 }
#endif
	MPT_STRUCT(array) _d;
	uint16_t _flags;
	uint8_t _type;
	uint8_t _esize;
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
MPT_INTERFACE(rawdata)
{
public:
	virtual int modify(unsigned , int , const void *, size_t , size_t, int = -1) = 0;
	virtual int advance() = 0;
	
	virtual int values(unsigned , struct iovec * = 0, int = -1) const = 0;
	virtual int dimensions(int = -1) const = 0;
	virtual int stages() const = 0;
	
	static int typeIdentifier();
protected:
	inline ~rawdata() { }
};
#else
MPT_INTERFACE(rawdata);
MPT_INTERFACE_VPTR(rawdata) {
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
		bool set(long);
		long userLength();
		long rawLength();
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
	
	static int typeIdentifier();
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
extern void mpt_values_linear(long , double *, long , double , double);
extern void mpt_values_bound (long , double *, long , double , double , double);
extern long mpt_values_iter(long , double *, long , MPT_INTERFACE(iterator) *);

/* prepare values on buffer offset */
extern double *mpt_values_prepare(_MPT_ARRAY_TYPE(double) *, long);

/* create profile for grid */
extern MPT_INTERFACE(metatype) *mpt_iterator_profile(const _MPT_ARRAY_TYPE(double) *, const char *);

/* create iterators for input */
extern MPT_INTERFACE(metatype) *mpt_iterator_linear(uint32_t , double , double);
extern MPT_INTERFACE(metatype) *mpt_iterator_boundary(uint32_t , double , double , double);
extern MPT_INTERFACE(metatype) *mpt_iterator_poly(const char *, const _MPT_ARRAY_TYPE(double) *);
extern MPT_INTERFACE(metatype) *mpt_iterator_file(int);
extern MPT_INTERFACE(metatype) *mpt_iterator_values(const char *);

/* create iterator (descr. includes type info) */
extern MPT_INTERFACE(metatype) *mpt_iterator_create(const char *);
/* create specific iterator */
extern MPT_INTERFACE(metatype) *_mpt_iterator_range (MPT_STRUCT(value) *);
extern MPT_INTERFACE(metatype) *_mpt_iterator_linear(MPT_STRUCT(value) *);
extern MPT_INTERFACE(metatype) *_mpt_iterator_factor(MPT_STRUCT(value) *);

#if defined(_STDIO_H) || defined(_STDIO_H_)
/* set solver matrix via file */
extern int mpt_values_file(FILE *, long , long , double *);
#endif


/* multi dimension data operations */
extern MPT_STRUCT(typed_array) *mpt_stage_data(MPT_STRUCT(rawdata_stage) *, unsigned);
extern void mpt_stage_fini(MPT_STRUCT(rawdata_stage) *);
/* set dimensions to defined size */
extern ssize_t mpt_stage_truncate(MPT_STRUCT(rawdata_stage) *, size_t __MPT_DEFPAR(0));

/* create cycle interface with default data parts */
extern MPT_INTERFACE(metatype) *mpt_rawdata_create(size_t);
extern int mpt_rawdata_typeid(void);

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
extern int mpt_fpoint_set(MPT_STRUCT(fpoint) *, const MPT_INTERFACE(metatype) *, const MPT_STRUCT(range) *__MPT_DEFPAR(0));

/* consume range data */
extern int mpt_range_set(MPT_STRUCT(range) *, MPT_STRUCT(value) *);

#ifdef _MPT_MESSAGE_H
/* data mapping operations */
extern int mpt_mapping_add(_MPT_ARRAY_TYPE(mapping) *, const MPT_STRUCT(mapping) *);
extern int mpt_mapping_del(const _MPT_ARRAY_TYPE(mapping) *, const MPT_STRUCT(msgbind) *, const MPT_STRUCT(msgdest) * __MPT_DEFPAR(0), int __MPT_DEFPAR(0));
extern int mpt_mapping_cmp(const MPT_STRUCT(mapping) *, const MPT_STRUCT(msgbind) *, int __MPT_DEFPAR(0));
#endif

__MPT_EXTDECL_END

#ifdef __cplusplus
size_t maxsize(Slice<const typed_array>, int = -1);

inline linepart::linepart(int usr, int raw) : raw(raw >= 0 ? raw : usr), usr(usr), _cut(0), _trim(0)
{ }

inline void *typed_array::reserve(size_t off, size_t len)
{ return mpt_array_slice(&_d, off, len); }

class Transform
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
# if __cplusplus >= 201103L
	using Point = point<double>;
# else
	struct Point : public point<double>
	{
	public:
		inline Point(const point<double> &from = point<double>(0, 0)) : point<double>(from)
		{ }
	};
# endif
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
	Array<Point> _values;
};
template<> inline int typeIdentifier<Polyline::Point>()
{
	return typeIdentifier<point<double> >();
}

class Cycle : public reference, public rawdata
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
	/* reference interface */
	virtual uintptr_t addref();
	void unref() __MPT_OVERRIDE;
	
	/* basic raw data interface */
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

template<> inline int typeIdentifier<rawdata>()
{
	return rawdata::typeIdentifier();
}

#endif

__MPT_NAMESPACE_END

#endif /* _MPT_VALUES_H */
