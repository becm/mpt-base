/*!
 * MPT plotting library
 *  value creation and processing
 */

#ifndef _MPT_VALUES_H
#define _MPT_VALUES_H  @INTERFACE_VERSION@

#include "array.h"

__MPT_NAMESPACE_BEGIN

MPT_INTERFACE(metatype);
MPT_INTERFACE(output);

MPT_STRUCT(node);

/* primitive type point/transformation structure */
#ifdef __cplusplus
class transform;

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
template<> int typeinfo<point<double> >::id();

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

/*! layout destination */
MPT_STRUCT(laydest)
{
#ifdef __cplusplus
	inline laydest(uint8_t l = 0, uint8_t g = 0, uint8_t w = 0, uint8_t d = 0) :
		lay(l), grf(g), wld(w), dim(d)
	{ }
	enum {
		MatchLayout    = 1,
		MatchGraph     = 2,
		MatchWorld     = 4,
		MatchPath      = MatchLayout | MatchGraph | MatchWorld,
		MatchDimension = 8,
		MatchAll       = -1
	};
	bool match(laydest dst, int = MatchAll) const;
	
	inline bool operator ==(const laydest &cmp)
	{
		return match(cmp);
	}
#else
# define MPT_LAYDEST_INIT { 0, 0, 0, 0 }
#endif
	uint8_t lay,  /* target layout */
	        grf,  /* target graph */
	        wld,  /* target world */
	        dim;  /* target dimension */
};

/*! value destination */
MPT_STRUCT(valdest)
{
#ifdef __cplusplus
	inline valdest() : cycle(0), offset(0) { }
#else
# define MPT_VALDEST_INIT { 0, 0 }
#endif
	uint32_t cycle,   /* target cycle */
	         offset;  /* data offset */
};

/*! value source */
#ifdef __cplusplus
MPT_STRUCT(valsrc)
{
	enum {
#define MPT_DATASTATE(x)  x
#else
enum MPT_ENUM(DataStates) {
#define MPT_DATASTATE(x)  MPT_ENUM(DataState##x)
#endif
	MPT_DATASTATE(Init)  = 0x1,   /* data states */
	MPT_DATASTATE(Step)  = 0x2,
	MPT_DATASTATE(Fini)  = 0x4,
	MPT_DATASTATE(Fail)  = 0x8,
	MPT_DATASTATE(All)   = 0x7
};
#ifdef __cplusplus
	inline valsrc(int d, int s = Init | Step) : dim(d), state(s)
	{ }
#else
MPT_STRUCT(valsrc)
{
# define MPT_VALSRC_INIT { 0, (MPT_DATASTATE(Init) | MPT_DATASTATE(Step)) }
#endif
	uint8_t dim,    /* source dimension */
	        state;  /* context of data */
};

/*! information about containing data */
MPT_STRUCT(value_store)
{
#ifdef __cplusplus
public:
	enum Flags {
		ValueChange = 1
	};
	inline value_store() : _flags(0), _code(0)
	{ }
	void *reserve(int , size_t , long = 0);
	void set_modified(bool mod = true);
	size_t element_size() const;
	long element_count() const;
	int type() const;
	
	inline int flags() const
	{
		return _flags;
	}
	inline const void *base() const
	{
		const array::content *d = _d.data();
		return d ? (d + 1) : 0;
	}
protected:
#else
# define MPT_VALUE_STORE_INIT { MPT_ARRAY_INIT, 0, 0 }
#endif
	MPT_STRUCT(array) _d;
	uint16_t _flags;
	uint8_t  _code;
};
#ifdef __cplusplus
template<> int typeinfo<value_store>::id();
#endif

MPT_STRUCT(rawdata_stage)
{
#ifdef __cplusplus
    public:
	int clear_modified(int = -1);
	
	value_store *values(long dim);
	inline span<const value_store> values() const
	{
		return _d.elements();
	}
	inline long dimension_count() const
	{
		return _d.length();
	}
    protected:
#else
# define MPT_RAWDATA_STAGE_INIT { MPT_ARRAY_INIT, 0 }
#endif
	_MPT_ARRAY_TYPE(value_store) _d;
	uint8_t _max_dimensions;
};
#ifdef __cplusplus
template<> int typeinfo<rawdata_stage>::id();
#endif

/* part of MPT world */
#ifdef __cplusplus
MPT_INTERFACE(rawdata)
{
public:
	virtual int modify(unsigned , int , const void *, size_t , const valdest * = 0) = 0;
	virtual int advance() = 0;
	
	virtual int values(unsigned , struct iovec * = 0, int = -1) const = 0;
	virtual long dimension_count(int = -1) const = 0;
	virtual long stage_count() const = 0;
protected:
	inline ~rawdata() { }
};
template<> int typeinfo<rawdata>::id();
#else
MPT_INTERFACE(rawdata);
MPT_INTERFACE_VPTR(rawdata) {
	int (*modify)(MPT_INTERFACE(rawdata) *, unsigned , int , const void *, size_t , const MPT_STRUCT(valdest) *);
	int (*advance)(MPT_INTERFACE(rawdata) *);
	
	int (*values)(const MPT_INTERFACE(rawdata) *, unsigned , struct iovec *, int);
	int (*dimension_count)(const MPT_INTERFACE(rawdata) *, int);
	int (*stage_count)(const MPT_INTERFACE(rawdata) *);
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
	
	bool set_trim(float);
	bool set_cut(float);
	
	class array : public typed_array<linepart>
	{
	public:
		bool apply(const transform &, int , span<const double>);
		bool set(long);
		long length_user();
		long length_raw();
	};
#endif
	uint16_t raw,   /* raw points in line part */
	         usr,   /* transformed points in line part */
	        _cut,
	        _trim;  /* remove fraction from line start/end */
};

MPT_STRUCT(value_apply)
{
#ifdef __cplusplus
	value_apply(int = -1);
	
	void set(const struct range &, int);
#endif
	double             scale;  /* raw value scale factor */
	float              add;    /* scaled value offset */
	uint32_t          _flags;  /* axis and transformation flags */
	MPT_STRUCT(fpoint) to;     /* apply (add + scale * <raw>) to X and Y components */
};

/* binding to layout mapping */
MPT_STRUCT(mapping)
{
#ifdef __cplusplus
	inline mapping(const valsrc &m = valsrc(0), const laydest &d = laydest(), int c = 0) :
		src(m), client(c), dest(d)
	{ }
	inline bool valid() const
	{
		return src.state != 0;
	}
#else
# define MPT_MAPPING_INIT { MPT_VALSRC_INIT, 0, MPT_LAYDEST_INIT }
#endif
	MPT_STRUCT(valsrc)  src;
	uint16_t            client;
	MPT_STRUCT(laydest) dest;
};

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

/* type identifiers for special array and stage data */
extern int mpt_value_store_typeid(void);
extern int mpt_rawdata_stage_typeid(void);
/* multi dimension data operations */
extern MPT_STRUCT(value_store) *mpt_stage_data(MPT_STRUCT(rawdata_stage) *, unsigned);
extern void mpt_stage_fini(MPT_STRUCT(rawdata_stage) *);
/* reserve data for value segment */
extern void *mpt_value_store_reserve(MPT_STRUCT(array) *, int , size_t, long);
/* set dimensions to defined size */
extern ssize_t mpt_stage_truncate(MPT_STRUCT(rawdata_stage) *, size_t __MPT_DEFPAR(0));

/* create cycle interface with default data parts */
extern MPT_INTERFACE(metatype) *mpt_rawdata_create(long __MPT_DEFPAR(-1));
extern int mpt_rawdata_typeid(void);

/* create linear part in range */
extern void mpt_linepart_linear(MPT_STRUCT(linepart) *, const double *, size_t , const MPT_STRUCT(range) *);
/* encode/decode linepart trim and cut values */
extern int mpt_linepart_code(double val);
extern double mpt_linepart_real(int val);
/* join parts if allowed by size and no userdata in second */
extern MPT_STRUCT(linepart) *mpt_linepart_join(MPT_STRUCT(linepart) *, const MPT_STRUCT(linepart));

/* initialize transform dimension */
extern void mpt_value_apply_init(MPT_STRUCT(value_apply) *, int __MPT_DEFPAR(-1));

/* set point data */
extern int mpt_fpoint_set(MPT_STRUCT(fpoint) *, const MPT_INTERFACE(metatype) *, const MPT_STRUCT(range) *__MPT_DEFPAR(0));

/* consume range data */
extern int mpt_range_set(MPT_STRUCT(range) *, MPT_STRUCT(value) *);

/* data mapping operations */
extern int mpt_mapping_add(_MPT_ARRAY_TYPE(mapping) *, const MPT_STRUCT(mapping) *);
extern int mpt_mapping_del(const _MPT_ARRAY_TYPE(mapping) *, const MPT_STRUCT(valsrc) *, const MPT_STRUCT(laydest) * __MPT_DEFPAR(0), int __MPT_DEFPAR(0));
extern int mpt_mapping_cmp(const MPT_STRUCT(mapping) *, const MPT_STRUCT(valsrc) *, int __MPT_DEFPAR(0));

/* set value source state for description */
extern int mpt_valsrc_state(MPT_STRUCT(valsrc) *, const char *);

/* push bindings to output */
extern int mpt_output_bind_list(MPT_INTERFACE(output) *, const MPT_STRUCT(node) *);
extern int mpt_output_bind_string(MPT_INTERFACE(output) *, const char *);

/* push message value type and destination header to output */
extern int mpt_output_init_plot(MPT_INTERFACE(output) *, MPT_STRUCT(laydest), uint8_t , const MPT_STRUCT(valdest) * __MPT_DEFPAR(0));

/* send layout open command */
extern int mpt_layout_open(MPT_INTERFACE(output) *, const char *, const char *);

__MPT_EXTDECL_END

#ifdef __cplusplus
size_t maxsize(span<const value_store>, int = -1);

inline linepart::linepart(int usr, int raw) : raw(raw >= 0 ? raw : usr), usr(usr), _cut(0), _trim(0)
{ }

class transform : public instance
{
public:
	void unref() __MPT_OVERRIDE;
	
	virtual int dimensions() const = 0;
	
	virtual linepart part(unsigned , const double *, int) const;
	virtual bool apply(unsigned , const linepart &, point<double> *, const double *) const;
	virtual point<double> zero() const;
protected:
	inline virtual ~transform()
	{ }
};

extern int apply_data(point<double> *, const span<const linepart> &, const transform &, span<const value_store>);
class polyline
{
public:
# if __cplusplus >= 201103L
	using point = ::mpt::point<double>;
# else
	struct point : public ::mpt::point<double>
	{
	public:
		inline point(const ::mpt::point<double> &from = ::mpt::point<double>(0, 0)) : ::mpt::point<double>(from)
		{ }
	};
# endif
	class part
	{
	public:
		part(const linepart lp, const point *pts) : _part(lp), _pts(pts)
		{ }
		span<const point> line() const;
		span<const point> points() const;
	protected:
		linepart _part;
		const point *_pts;
	};
	class iterator
	{
	public:
		iterator(span<const linepart> l, const point *p) : _parts(l), _points(p)
		{ }
		part operator *() const;
		iterator & operator++ ();
		
		bool operator== (const iterator &it) const
		{
			return _points == it._points;
		}
		inline bool operator!= (const iterator &it) const
		{
			return !operator==(it);
		}
	protected:
		span<const linepart> _parts;
		const point *_points;
	};
	bool set(const transform &, const rawdata &, int = -1);
	bool set(const transform &, span<const value_store>);
	
	iterator begin() const;
	iterator end() const;
	
	inline void clear()
	{
		_vis.set(0);
		_values.resize(0);
	}
	span<const linepart> parts() const
	{
		return _vis.elements();
	}
	span<const point> points() const
	{
		return _values.elements();
	}
protected:
	linepart::array _vis;
	typed_array<point> _values;
};

template<> int typeinfo<polyline::point>::id();

class cycle : public instance, public rawdata
{
public:
	enum Flags {
		LimitStages = 1
	};
	cycle();
	
	class stage : public rawdata_stage
	{
	public:
		inline const polyline &values() const
		{
			return _values;
		}
		inline void invalidate()
		{
			_values.clear();
		}
		bool transform(const class transform &);
	protected:
		polyline _values;
	};
	/* instance interface */
	void unref() __MPT_OVERRIDE;
	
	/* basic raw data interface */
	int modify(unsigned , int , const void *, size_t , const valdest * = 0) __MPT_OVERRIDE;
	int advance() __MPT_OVERRIDE;
	
	int values(unsigned , struct iovec * = 0, int = -1) const __MPT_OVERRIDE;
	long dimension_count(int = -1) const __MPT_OVERRIDE;
	long stage_count() const __MPT_OVERRIDE;
	
	virtual void limit_dimensions(uint8_t);
	virtual bool limit_stages(size_t);
	
	inline stage *begin()
	{
		return _stages.begin();
	}
	inline stage *end()
	{
		return _stages.end();
	}
	inline span<const stage> stages() const
	{
		return _stages.elements();
	}
protected:
	virtual ~cycle();
	typed_array<stage> _stages;
	uint16_t _act;
	uint8_t _max_dimensions;
	uint8_t _flags;
};

template <typename T, typename S>
void apply(T *d, const linepart &pt, const S *src, const T &scale)
{
	size_t j = 0, len = pt.usr;
	S sx(scale.x), sy(scale.y), part;
	
	if ((part = pt.cut())) {
		if (pt.raw < 2) {
			return;
		}
		++j;
		S s1 = src[0];
		S s2 = src[1];
		part = s1 + part * (s2 - s1);
		
		d->x += sx * part;
		d->y += sy * part;
	}
	if ((part = pt.trim())) {
		if (pt.raw < 2) {
			return;
		}
		--len;
		S s1 = src[len - 1];
		S s2 = src[len];
		part = s2 + part * (s1 - s2);
		
		d[len].x += sx * part;
		d[len].y += sy * part;
	}
	for ( ; j < len ; j++ ) {
		d[j].x += sx * src[j];
		d[j].y += sy * src[j];
	}
}

#endif

__MPT_NAMESPACE_END

#endif /* _MPT_VALUES_H */
