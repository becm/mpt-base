/*!
 * MPT core library
 *  resizable data segments
 */

#ifndef _MPT_ARRAY_H
#define _MPT_ARRAY_H  @INTERFACE_VERSION@

#include <stdarg.h>

#ifdef __cplusplus
# include <new>
# include <cstring>
# include <cstdlib>
# include "output.h"
# include "types.h"
#else
# include "core.h"
#endif

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(slice);
MPT_STRUCT(type_traits);

MPT_INTERFACE(metatype);

/* tree and list operation flags */
enum MPT_ENUM(BufferFlags) {
	/* capabilities */
	MPT_ENUM(BufferNoCopy)       = 0x02,
	MPT_ENUM(BufferImmutable)    = 0x01,
	
	MPT_ENUM(BufferFlagsUser)    = 0xff,
	
	/* dynamic state flags */
	MPT_ENUM(BufferShared)       = 0x0100,
	
	/* static memory information */
	MPT_ENUM(BufferMapped)       = 0x010000
};

/*! header for data segment */
#ifdef __cplusplus
MPT_STRUCT(buffer)
{
public:
	virtual uint32_t get_flags() const = 0;
	virtual void unref() = 0;
	virtual uintptr_t addref() = 0;
	virtual buffer *detach(size_t) = 0;
	
	bool copy(const buffer &);
	
	bool move(buffer &);
	bool skip(size_t);
	bool trim(size_t);
	
	void *append(size_t);
	void *insert(size_t , size_t);
	
	inline const struct type_traits *content_traits() const
	{
		return _content_traits;
	}
	inline bool shared() {
		return (get_flags() & BufferShared) != 0;
	}
	inline bool immutable() {
		return (get_flags() & BufferImmutable) != 0;
	}
	static buffer *create(size_t , const struct type_traits * = 0);
	static buffer *create_unique(size_t , const struct type_traits * = 0);
protected:
	inline buffer(size_t size = 0) : _content_traits(0), _size(size), _used(0)
	{ }
	inline ~buffer()
	{ }
#else
MPT_STRUCT(buffer);
MPT_INTERFACE_VPTR(buffer)
{
	uint32_t (*get_flags)(const MPT_STRUCT(buffer) *);
	void (*unref)(MPT_STRUCT(buffer) *);
	uintptr_t (*addref)(MPT_STRUCT(buffer) *);
	MPT_STRUCT(buffer) *(*detach)(MPT_STRUCT(buffer) *, size_t);
}; MPT_STRUCT(buffer)
{
	const MPT_INTERFACE_VPTR(buffer) *_vptr;
#endif
	const MPT_STRUCT(type_traits) *_content_traits;
	const size_t _size;
	size_t _used;
};

/*! reference to buffer data */
MPT_STRUCT(array)
{
#ifdef __cplusplus
	class content : public buffer
	{
	public:
		inline size_t length() const
		{
			return _used;
		}
		inline size_t left() const
		{
			return _size - _used;
		}
		inline void *data() const
		{
			return static_cast<void *>(const_cast<content *>(this) + 1);
		}
		bool set_length(size_t);
	protected:
		inline ~content()
		{ }
	};
	array(array const&);
	array(size_t = 0);
	
	const content *data() const;
	
	size_t length() const;
	size_t left() const;
	void *base() const;
	bool shared() const;
	
	char *string();
	bool compact();
	
	void *append(size_t , const void * = 0);
	void *prepend(size_t , const void * = 0);
	void *insert(size_t , size_t , const void * = 0);
	void *set(size_t , const void * = 0);
	bool set(const reference<buffer> &);
	
	int set(const value &);
	int set(convertable &);
	int printf(const char *fmt, ... );
	
	array & operator=  (const array &);
	array & operator=  (const slice &);
	array & operator+= (const content &);
	array & operator+= (const span<uint8_t> &);
	array & operator=  (struct ::iovec const&);
	array & operator+= (struct ::iovec const&);
protected:
# define _MPT_ARRAY_TYPE(x)     ::mpt::typed_array<x>
# define _MPT_UARRAY_TYPE(x)    ::mpt::unique_array<x>
# define _MPT_REF_ARRAY_TYPE(x) ::mpt::reference_array<x>
	reference<content> _buf;
#else
	MPT_STRUCT(buffer) *_buf;
# define _MPT_ARRAY_TYPE(x)     MPT_STRUCT(array)
# define _MPT_UARRAY_TYPE(x)    MPT_STRUCT(array)
# define _MPT_REF_ARRAY_TYPE(x) MPT_STRUCT(array)
# define MPT_ARRAY_INIT { 0 }
#endif
};
/*! transforming appended data */
MPT_STRUCT(encode_array)
{
#ifdef __cplusplus
public:
	encode_array(data_encoder_t = 0);
	~encode_array();
	
	bool prepare(size_t);
	ssize_t push(size_t , const void *);
	bool set_encoding(data_encoder_t);
	bool shift(size_t = 0);
	
	bool push(const struct message &);
	
	span<const uint8_t> data() const;
protected:
#else
# define MPT_ENCODE_ARRAY_INIT { 0, MPT_ENCODE_INIT, MPT_ARRAY_INIT }
#endif
	MPT_TYPE(data_encoder) _enc;
	MPT_STRUCT(encode_state) _state;
	MPT_STRUCT(array) _d;
};


#ifdef __cplusplus
template<> inline __MPT_CONST_TYPE int type_properties<buffer *>::id(bool) {
	return TypeBufferPtr;
}
template<> inline const type_traits *type_properties<buffer *>::traits() {
	static const type_traits *traits = 0;
	return traits ? traits : (traits = type_traits::get(id(true)));
}

template<> inline __MPT_CONST_TYPE int type_properties<array>::id(bool) {
	return TypeArray;
}
template<> inline const type_traits *type_properties<array>::traits() {
	static const type_traits *traits = 0;
	return traits ? traits : (traits = type_traits::get(id(true)));
}

/*! reference to buffer segment */
struct slice : array
{
	slice(const array &);
	slice(const slice &);
	slice(array::content * = 0);
	~slice();
	
	span<const uint8_t> data() const;
	
	ssize_t write(size_t , const void *, size_t);
	int set(convertable &);
	
	bool shift(ssize_t);
	bool trim (ssize_t);
protected:
#else
MPT_STRUCT(slice)
{
# define MPT_SLICE_INIT { MPT_ARRAY_INIT, 0, 0 }
	MPT_STRUCT(array) _a;
#endif
	uintptr_t _off, /* start position on array */
	          _len; /* used data size on array */
};

#ifdef __cplusplus
template<typename MPT_RANGE_T>
# define MPT_RANGE_FCN range
#endif
#if defined(MPT_RANGE_FCN) && (defined(__cplusplus) || defined(MPT_RANGE_T))
extern void MPT_RANGE_FCN(MPT_RANGE_T range[2], int len, const MPT_RANGE_T *val, int ld)
{
	if (!len) {
#ifdef __cplusplus
		range[0] = range[1] = MPT_RANGE_T();
#else
		range[0] = range[1] = 0;
#endif
		return;
	}
	range[0] = range[1] = *val;
	
	if (!ld) {
		return;
	}
	while (--len > 0) {
		val += ld;
		if (*val < range[0]) {
			range[0] = *val;
		}
		if (*val > range[1]) {
			range[1] = *val;
		}
	}
}
#endif

#ifdef __cplusplus
template<typename MPT_COPY_ST, typename MPT_COPY_DT>
# define MPT_COPY_FCN copy
#endif
#if defined(MPT_COPY_FCN) && (defined(__cplusplus) || (defined(MPT_COPY_ST) && defined(MPT_COPY_DT)))
extern void MPT_COPY_FCN(int pts, const MPT_COPY_ST *src, int lds, MPT_COPY_DT *dest, int ldd)
{
	int i, j;
	
	if (pts <= 0) {
		return;
	}
	if (!ldd) {
		dest[0] = src[(pts - 1) * lds];
		return;
	}
	if (!lds) {
		for (i = 0; i < pts; i++) {
			dest[i] = src[0];
		}
		return;
	}
	if (lds == 1 && ldd == 1) {
		for (i = 0; i < pts; i++) {
			dest[i] = src[i];
		}
		return;
	}
	for (i = 0, j = 0; pts--; i += ldd, j += lds) {
		dest[i] = src[j];
	}
}
#endif

#ifdef __cplusplus
template<typename SRC, typename DST>
extern void copy(long pts, const SRC *src, DST *dest)
{
	for (long i = 0; i < pts; i++) {
		dest[i] = src[i];
	}
}
template<> void copy<double, double>(long pts, const double *, double *);
#endif

__MPT_EXTDECL_BEGIN

/* array type information */
extern const MPT_STRUCT(type_traits) *mpt_array_traits(void);

/* get range of array */
extern void mpt_drange(double  [2], int , const double  *, int);
extern void mpt_frange(float   [2], int , const float   *, int);
extern void mpt_irange(int32_t [2], int , const int32_t *, int);

/* copy/convert operations with leading dimension */
#if !defined(MPT_COPY_ST) && !defined(MPT_COPY_DT)
extern void mpt_copy64(int , const void *, int , void *, int);
extern void mpt_copy32(int , const void *, int , void *, int);

extern void mpt_copy_fd(int , const float  *, int , double *, int);
extern void mpt_copy_df(int , const double *, int , float  *, int);
#endif

/* iterator with buffer data */
extern MPT_INTERFACE(metatype) *mpt_meta_buffer(const MPT_STRUCT(array) *);
/* treat first string segment as metatype value */
extern MPT_INTERFACE(metatype) *mpt_meta_arguments(const MPT_STRUCT(array) *);

/* array manipulation */
extern size_t mpt_array_reduce(MPT_STRUCT(array) *);
extern int mpt_array_clone(MPT_STRUCT(array) *, const MPT_STRUCT(array) *);

/* add data to array */
extern void *mpt_array_append(MPT_STRUCT(array) *, size_t , const void *__MPT_DEFPAR(0));
extern void *mpt_array_insert(MPT_STRUCT(array) *, size_t , size_t);
extern void *mpt_array_set(MPT_STRUCT(array) *, const MPT_STRUCT(type_traits) *, size_t , const void *, long __MPT_DEFPAR(0));
/* prime array for data use */
extern MPT_STRUCT(buffer) *mpt_array_reserve(MPT_STRUCT(array) *, size_t , const MPT_STRUCT(type_traits) *);

/* insert data into buffer */
extern void *mpt_buffer_insert(MPT_STRUCT(buffer) *, size_t , size_t);
/* remove data from buffer */
extern ssize_t mpt_buffer_cut(MPT_STRUCT(buffer) *, size_t , size_t);
/* copy buffer content */
extern long mpt_buffer_set(MPT_STRUCT(buffer) *, const MPT_STRUCT(type_traits) *, size_t , const void *, size_t);
/* get data element */
extern void *mpt_buffer_data(const MPT_STRUCT(buffer) *, size_t , size_t);

/* create and return slice data */
extern void *mpt_array_slice(MPT_STRUCT(array) *, size_t , size_t);

/* write data to slice */
extern ssize_t mpt_slice_write(MPT_STRUCT(slice) *, size_t , const void *, size_t);

/* advance slice element */
extern int mpt_slice_next(MPT_STRUCT(slice) *);

/* snprintf to to array */
extern int mpt_printf(MPT_STRUCT(array) *, const char *, ... );
#if !defined(__STRICT_ANSI__) || __STDC_VERSION__ + 0 >= 199900L || defined(__cplusplus)
extern int mpt_vprintf(MPT_STRUCT(array) *, const char *, va_list );
#endif
/* return zero-terminated buffer data */
extern char *mpt_array_string(MPT_STRUCT(array) *);

/* add message to array */
extern ssize_t mpt_array_push(MPT_STRUCT(encode_array) *, size_t len, const void *data);
/* finalize encode array content */
extern void mpt_encode_array_fini(MPT_STRUCT(encode_array) *);

/* pointer/metatype array */
extern long mpt_array_compact(void **, long);
extern size_t mpt_array_move(void *, size_t , size_t , size_t);

/* buffer resizing backends */
extern MPT_STRUCT(buffer) *_mpt_buffer_alloc(size_t, int __MPT_DEFPAR(0));
extern MPT_STRUCT(buffer) *_mpt_buffer_map(size_t, int __MPT_DEFPAR(0));

/* bit operations */
extern int mpt_bitmap_set(uint8_t *, size_t , long);
extern int mpt_bitmap_unset(uint8_t *, size_t , long);
extern int mpt_bitmap_get(const uint8_t *, size_t , long);

__MPT_EXTDECL_END

#ifdef __cplusplus
inline void *array::base() const
{
	content *d = _buf.instance();
	if (!d || d->content_traits()) {
		return 0;
	}
	return d->data();
}
inline size_t array::length() const
{
	content *d = _buf.instance();
	return (d && !d->content_traits()) ? d->length() : 0;
}
inline size_t array::left() const
{
	content *d = _buf.instance();
	return (d && !d->content_traits()) ? d->left() : 0;
}
inline bool array::shared() const
{
	content *d = _buf.instance();
	return d && d->shared();
}
inline const array::content *array::data() const
{
	return _buf.instance();
}
inline void *array::prepend(size_t len, const void *data)
{
	return insert(0, len, data);
}
inline array &array::operator= (slice const& from)
{
	span<const uint8_t> d = from.data();
	set(d.size(), d.begin());
	return *this;
}
inline array &array::operator+= (content const& from)
{
	append(from.length(), from.data());
	return *this;
}
inline array &array::operator+= (span<uint8_t> const& from)
{
	append(from.size(), from.begin());
	return *this;
}
inline slice::slice(array const& a) : array(a), _off(0)
{
	_len = length();
}
inline slice::~slice()
{ }
inline span<const uint8_t> slice::data() const
{
	buffer *b = _buf.instance();
	return span<const uint8_t>(b ? ((uint8_t *) (b + 1)) + _off : 0, _len);
}

long compact(span<void *>);
long unused(span<void *>);
template <typename T>
bool swap(span<T> s, long p1, long p2)
{
	long len = s.size();
	if (p1 > len || p2 > len) {
		return false;
	}
	T *b = s.begin();
	T t = b[p1];
	b[p1] = b[p2];
	b[p2] = t;
	return true;
}
template <typename T>
long offset(const span<T> &s, const T &ref)
{
	long pos = 0;
	for (const T *c = s.begin(), *e = s.end(); c != e; c++) {
		if (*c == ref) {
			return pos;
		}
		++pos;
	}
	return -1;
}

template <typename T>
void move(T *v, long from, long to)
{
	if (from == to) {
		return;
	}
	/* save data to anonymous store */
	uint8_t buf[sizeof(T)];
	memcpy(buf, v + from, sizeof(T));
	/* move data between positions */
	if (from < to) {
		memmove((void *) (v + from), (void *) (v + from + 1), (to - from) * sizeof(*v));
	} else {
		memmove((void *) (v + to + 1), (void *) (v + to),     (from - to) * sizeof(*v));
	}
	/* restore data to new position */
	memcpy(v + to, buf, sizeof(*v));
}

template <typename T>
class content : public buffer
{
public:
	typedef T* iterator;
	
	inline iterator begin() const
	{
		return static_cast<T *>(const_cast<void *>(static_cast<const void *>(this + 1)));
	}
	inline iterator end() const
	{
		return begin() + length();
	}
	inline span<T> data() const
	{
		return span<T>(begin(), length());
	}
	
	content<T> *detach(size_t len) __MPT_OVERRIDE = 0;
	
	inline long length() const
	{
		return _used / sizeof(T);
	}
	inline long left() const
	{
		return (_size - _used) / sizeof(T);
	}
	bool set_length(long len)
	{
		size_t set = len * sizeof(T);
		if (set == _used) {
			return true;
		}
		if (set < _used) {
			return buffer::trim(_used - set);
		}
		/* data before offset gets initialized */
		uint8_t *ptr = static_cast<uint8_t *>(buffer::insert(set, 0));
		return ptr ? true : false;
	}
	void *insert(long pos)
	{
		if (pos < 0) {
			pos += _used / sizeof(T);
			if (pos < 0) {
				return 0;
			}
		}
		/* inserted data is NOT initialized */
		return buffer::insert(pos * sizeof(T), sizeof(T));
	}
	static content<T> *create(long len)
	{
		return static_cast<content<T> *>(buffer::create(len * sizeof(T), type_properties<T>::traits()));
	}
protected:
	inline content(unsigned int len, const type_traits &traits = *type_properties<T>::traits()) : buffer(len * sizeof(T))
	{
		_content_traits = &traits;
	}
	inline ~content()
	{ }
};

/*! typed array with standard operations */
template <typename T>
class unique_array
{
public:
	typedef T* iterator;
	
	inline unique_array(const unique_array &a) : _ref(0)
	{
		_ref = a._ref;
	}
	unique_array(long len = -1)
	{
		static default_data _dummy;
		content<T> *data = &_dummy;
		if (len >= 0) data = data->detach(len * sizeof(T));
		_ref.set_instance(data ? data : &_dummy);
	}
	inline iterator begin() const
	{
		content<T> *c = _ref.instance();
		return c ? c->begin() : 0;
	}
	inline iterator end() const
	{
		content<T> *c = _ref.instance();
		return c ? c->end() : 0;
	}
	inline unique_array & operator=(const unique_array &a)
	{
		_ref = a._ref;
		return *this;
	}
	inline unique_array & operator=(const reference<content<T> > &v)
	{
		this->_ref = v;
		return *this;
	}
	long offset(const T &ref) const
	{
		content<T> *data = _ref.instance();
		return data ? ::mpt::offset(data->data(), ref) : -1;
	}
	bool set(long pos, T const &v)
	{
		if (pos < 0) {
			if ((pos += length()) < 0) {
				return false;
			}
		}
		else if (pos >= length()) {
			return false;
		}
		if (!detach()) {
			return false;
		}
		T *t = begin();
		t[pos] = v;
		return true;
	}
	T *insert(long pos)
	{
		long len = length();
		if (pos < 0) {
			if ((pos += len) < 0) {
				return 0;
			}
		}
		else if (pos > len) {
			len = pos;
		}
		if (!reserve(len + 1)) {
			return 0;
		}
		content<T> *d = _ref.instance();
		/* inserted raw data is NOT initialized */
		void *ptr = d->insert(pos);
		if (!ptr) {
			return 0;
		}
		const struct type_traits *traits = d->content_traits();
		if (traits && traits->init && traits->init(ptr, 0) >= 0) {
			return static_cast<T *>(ptr);
		}
		return new (ptr) T;
	}
	T *get(long pos) const
	{
		if (pos < 0) {
			if ((pos += length()) < 0) {
				return 0;
			}
		}
		else if (pos >= length()) {
			return 0;
		}
		return begin() + pos;
	}
	inline long length() const
	{
		content<T> *d = _ref.instance();
		return d ? d->length() : 0;
	}
	inline span<const T> elements() const
	{
		return span<const T>(begin(), length());
	}
	bool detach()
	{
		content<T> *c, *n;
		if (!(c =  _ref.detach())) {
			return true;
		}
		long elem = c->length();
		if ((n = c->detach(elem * sizeof(T)))) {
			_ref.set_instance(n);
			return true;
		}
		_ref.set_instance(c);
		return false;
	}
	bool reserve(long len)
	{
		content<T> *c;
		if (!(c = _ref.detach())) {
			return false;
		}
		if (len < 0
		    && (len += length()) < 0) {
			_ref.set_instance(c);
			return false;
		}
		content<T> *n;
		if ((n = c->detach(len * sizeof(T)))) {
			_ref.set_instance(n);
			return true;
		}
		_ref.set_instance(c);
		return true;
	}
	bool resize(long len)
	{
		if (!reserve(len)) {
			return false;
		}
		content<T> *d;
		if (len >= 0 && (d = _ref.instance())) {
			return d->set_length(len);
		}
		return true;
	}
protected:
	class default_data : public content<T>
	{
	public:
		inline default_data(const type_traits &traits = *type_properties<T>::traits()) : content<T>(0, traits)
		{ }
		uint32_t get_flags() const __MPT_OVERRIDE
		{
			return BufferImmutable | BufferShared | BufferNoCopy;
		}
		void unref() __MPT_OVERRIDE
		{ }
		uintptr_t addref() __MPT_OVERRIDE
		{
			return 1;
		}
		content<T> *detach(size_t len) __MPT_OVERRIDE
		{
			size_t align = len % sizeof(T);
			return static_cast<content<T> *>(buffer::create_unique(len - align, this->content_traits()));
		}
	};
	inline unique_array(content<T> *ref) : _ref(ref)
	{ }
	reference<content<T> > _ref;
};
template<typename T>
class type_properties<unique_array<T> >
{
protected:
	type_properties();
public:
	static inline __MPT_CONST_EXPR int id(bool obtain) {
		return type_properties<array>::id(obtain);
	}
	static inline const MPT_STRUCT(type_traits) *traits(void) {
		return type_traits::get(id(true));
	}
};

template <typename T>
class typed_array : public unique_array<T>
{
public:
	typed_array(const typed_array &a) : unique_array<T>(a)
	{ }
	typed_array(long len = -1) : unique_array<T>(static_cast<content<T> *>(0))
	{
		static default_data _dummy;
		content<T> *data = &_dummy;
		if (len >= 0) data = data->detach(len * sizeof(T));
		this->_ref.set_instance(data ? data : &_dummy);
	}
	bool insert(long pos, const T &val)
	{
		long len = this->length();
		if (pos < 0) {
			if ((pos += len) < 0) {
				return 0;
			}
		}
		else if (pos > len) {
			len = pos;
		}
		if (!this->reserve(len + 1)) {
			return 0;
		}
		void *d = this->_ref.instance()->insert(pos);
		if (d) {
			new (d) T(val);
			return true;
		}
		return false;
	}
	inline typed_array & operator=(const typed_array &a)
	{
		this->_ref = a._ref;
		return *this;
	}
protected:
	class default_data : public unique_array<T>::default_data
	{
	public:
		content<T> *detach(size_t len) __MPT_OVERRIDE
		{
			size_t align = len % sizeof(T);
			return static_cast<content<T> *>(buffer::create(len - align, this->content_traits()));
		}
	};
};
template<typename T>
class type_properties<typed_array<T> >
{
protected:
	type_properties();
public:
	static inline __MPT_CONST_EXPR int id(bool obtain) {
		return type_properties<array>::id(obtain);
	}
	static const struct type_traits *traits()
	{
		return type_traits::get(id(true));
	}
};

/*! typed array with standard operations */
template <typename T>
class item_array : public unique_array<item<T> >
{
public:
	inline item_array(int len = -1) : unique_array<item<T> >(len)
	{ }
	inline item_array(const item_array &a) : unique_array<item<T> >(a)
	{ }
	inline item_array & operator=(const item_array &a)
	{
		this->_ref = a._ref;
		return *this;
	}
	item<T> *append(T *t, const char *id, int len = -1)
	{
		item<T> *it;
		long pos = this->length();
		if (!(it = this->insert(pos))) {
			return 0;
		}
		if (!id || it->identifier::set_name(id, len)) {
			it->set_instance(t);
			return it;
		}
		this->resize(pos);
		return 0;
	}
	long count() const
	{
		long len = 0;
		for (item<T> *pos = this->begin(), *to = this->end(); pos != to; ++pos) {
			if (pos->instance()) ++len;
		}
		return len;
	}
	bool compact()
	{
		item<T> *space = 0;
		long len = 0;
		for (item<T> *pos = this->begin(), *to = this->end(); pos != to; ++pos) {
			T *c = pos->instance();
			if (!c) {
				if (!space) space = pos;
				continue;
			}
			++len;
			if (!space) continue;
#if __cplusplus >= 201103L
			*space = std::move(*pos);
#else
			space->operator = (static_cast<const identifier &>(*pos));
			space->set_instance(pos->detach());
#endif
			pos->~item<T>();
			++space;
		}
		if (!space) {
			return false;
		}
		this->_ref.instance()->set_length(len);
		return true;
	}
};
template<typename T>
class type_properties<item_array<T> >
{
protected:
	type_properties();
public:
	static inline __MPT_CONST_EXPR int id(bool obtain) {
		return type_properties<array>::id(obtain);
	}
	static inline const struct type_traits *traits(void) {
		return type_traits::get(id(true));
	}
};

/*! basic pointer array */
template <typename T>
class pointer_array : public typed_array<T *>
{
public:
	typedef T** iterator;
	
	inline pointer_array(long len = 0) : typed_array<T *>(len)
	{ }
	inline pointer_array &operator =(typed_array<T *> &from)
	{
		this->_ref = from._ref;
		return *this;
	}
	inline span<void *> generic() const
	{
		T **data = this->begin();
		return span<void *>((void **) data, this->length());
	}
	inline void compact()
	{
		content<T *> *data = this->_ref.instance();
		if (data && !data->immutable()) {
			if (!data->shared()) {
				T **begin = data->begin();
				long length = ::mpt::compact(span<void *>((void **) begin, this->length()));
				data->set_length(length);
			}
			else {
				long length = this->length() - this->unused();
				content<T *> *next = content<T *>::create(length);
				for (T **b = data->begin(), **e = data->end(); b < e; ++b) {
					void *dest;
					if (*b && (dest = next->insert(next->length()))) {
						*static_cast<T **>(dest) = *b;
					}
				}
				this->_ref.set_instance(next);
			}
		}
	}
	inline long unused() const
	{
		return ::mpt::unused(generic());
	}
	inline bool swap(long p1, long p2) const
	{
		return ::mpt::swap(generic(), p1, p2);
	}
};
template<typename T>
class type_properties<pointer_array<T> >
{
protected:
	type_properties();
public:
	static inline __MPT_CONST_EXPR int id(bool obtain) {
		return type_properties<array>::id(obtain);
	}
	static inline const struct type_traits *traits(void) {
		return type_traits::get(id(true));
	}
};

/*! extendable array for reference types */
template <typename T>
class reference_array : public unique_array<reference<T> >
{
public:
	typedef reference<T>* iterator;
	
	inline reference_array(int len = -1) : unique_array<reference<T> >(static_cast<content<reference<T> > *>(0))
	{
		static class unique_array<reference<T> >::default_data _dummy(content_traits());
		content<reference<T> > *data = &_dummy;
		if (len >= 0) data = data->detach(len * sizeof(T));
		this->_ref.set_instance(data ? data : &_dummy);
	}
	bool insert(long pos, T *ref)
	{
		reference<T> *ptr = unique_array<reference<T> >::insert(pos);
		if (!ptr) {
			return false;
		}
		ptr->set_instance(ref);
		return true;
	}
	bool set(long pos, T *ref)
	{
		reference<T> *ptr = unique_array<reference<T> >::get(pos);
		if (!ptr) {
			return false;
		}
		ptr->set_instance(ref);
		return true;
	}
	long clear(const T *ref = 0) const
	{
		reference<T> *ptr = this->begin();
		long elem = 0;
		
		for (long i = 0, len = this->length(); i < len; ++i) {
			T *match = ptr[i].instance();
			if (!match || (ref && (match != ref))) {
				continue;
			}
			ptr[i].set_instance(0);
			++elem;
		}
		return elem;
	}
	long count() const
	{
		long len = 0;
		for (reference<T> *pos =  this->begin(), *to =  this->end(); pos != to; ++pos) {
			if (pos->instance()) ++len;
		}
		return len;
	}
	void compact()
	{
		::mpt::compact(span<void *>(reinterpret_cast<void **>(this->begin()), this->length()));
	}
protected:
	static const type_traits &content_traits() {
		static type_traits traits(sizeof(T), _unref_reference, 0);
		return traits;
	}
	static void _unref_reference(void *ptr)
	{
		static_cast<reference<T> *>(ptr)->set_instance(0);
	}
};
template<typename T>
class type_properties<reference_array<T> >
{
protected:
	type_properties();
public:
	static inline __MPT_CONST_EXPR int id(bool obtain) {
		return type_properties<array>::id(obtain);
	}
	static inline const MPT_STRUCT(type_traits) *traits(void) {
		return type_traits::get(id(true));
	}
};

class message_store : public logger, public reference<metatype>
{
public:
	enum {
		PassUnsaved =   0x1,
		PassSaved   =   0x2,
		PassFile    =   0x4,
		PassMessage =   0x8,
		PassFlags   =   0xf,
		
		SaveMessage =  0x10,
		SaveLog     =  0x20,
		SaveLogAll  =  0x40,
		SaveFlags   =  0xf0,
		
		FlowNormal  = PassUnsaved | PassSaved | PassFile
	};
	message_store(metatype * = 0);
	virtual ~message_store();
	
	class entry;
	
	int log(const char *, int, const char *, va_list) __MPT_OVERRIDE;
	
	virtual const entry *next();
	virtual void clear();
	
	virtual bool set_ignore_level(int);
	virtual bool set_flow_flags(int);
	
	inline int flow_flags() const
	{
		return _flags;
	}
	inline const span<const entry> messages() const
	{
		return _msg.elements();
	}
protected:
	typed_array<entry> _msg;
	uint32_t _act;
	uint8_t  _flags;
	uint8_t  _ignore;
	uint8_t  _level;
};
/*! storage for log messages */
class message_store::entry : array
{
public:
	logger::LogType type() const;
	const char *source() const;
	span<const char> data(int part = 0) const;
	int set(const char *, int, const char *, va_list);
protected:
	struct header
	{
		uint8_t from;
		uint8_t args;
		uint8_t _cmd;
		uint8_t type;
	};
};
template<> inline __MPT_CONST_TYPE int type_properties<message_store::entry>::id(bool obtain) {
	return type_properties<array>::id(obtain);
}
template<> inline const MPT_STRUCT(type_traits) *type_properties<message_store::entry>::traits() {
	return type_traits::get(id(true));
}

/*! linear search map type */
template <typename K, typename V>
class map
{
public:
	class entry
	{
	public:
		inline entry(const K &k = K(), const V &v = V()) : key(k), value(v)
		{ }
		K key;
		V value;
	};
	typedef const entry * const_iterator;
	
	inline const_iterator begin() const
	{
		return _d.begin();
	}
	inline const_iterator end() const
	{
		return _d.end();
	}
	bool set(const K &key, const V &value)
	{
		V *d = get(key);
		if (!d) {
			return _d.insert(_d.length(), entry(key, value));
		}
		*d = value;
		return true;
	}
	bool append(const K &key, const V &value)
	{
		return _d.insert(_d.length(), entry(key, value));
	}
	V *get(const K &key) const
	{
		for (entry *c = _d.begin(), *e = _d.end(); c < e; ++c) {
			if (c->key == key) {
				return &e->value;
			}
		}
		return 0;
	}
	inline typed_array<V> values(const K &key) const
	{
		return values(&key);
	}
	typed_array<V> values(const K *key = 0) const
	{
		typed_array<V> a;
		for (entry *c = _d.begin(), *e = _d.end(); c < e; ++c) {
			if (!key || c->key == *key) {
				a.insert(a.length(), c->value);
			}
		}
		return a;
	}
protected:
	typed_array<entry> _d;
};
template<typename K, typename V>
class type_properties<map<K, V> >
{
protected:
	type_properties();
public:
	static inline __MPT_CONST_EXPR int id(bool obtain) {
		return type_properties<array>::id(obtain);
	}
	static inline const MPT_STRUCT(type_traits) *traits(void) {
		return type_traits::get(id(true));
	}
};

#endif /* __cplusplus */

__MPT_NAMESPACE_END

#endif /* _MPT_ARRAY_H */
