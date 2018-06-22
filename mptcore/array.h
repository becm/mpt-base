/*!
 * MPT core library
 *  resizable data segments
 */

#ifndef _MPT_ARRAY_H
#define _MPT_ARRAY_H  @INTERFACE_VERSION@

#ifdef __cplusplus
# include <new>
# include <cstring>
# include <cstdlib>
# include "output.h"
#else
# include "core.h"
#endif

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(slice);

MPT_INTERFACE(metatype);

MPT_STRUCT(type_traits)
{
#ifdef __cplusplus
	inline type_traits() : init(0), copy(0), fini(0), type(0), size(0), base(0)
	{ }
#else
# define MPT_TYPETRAIT_INIT(t, i)  { 0, 0, 0,  (i), sizeof(t), (i) }
#endif
	void   (*init)(const MPT_STRUCT(type_traits) *, void *);
	int    (*copy)(void *, int , void *);
	void   (*fini)(void *);
	int      type;
	uint16_t size;
	uint8_t  base;
};

/*! header for data segment */
#ifdef __cplusplus
MPT_STRUCT(buffer) : public reference
{
public:
	virtual buffer *detach(size_t);
	virtual int shared() const = 0;
	
	bool copy(const buffer &);
	
	bool move(buffer &);
	bool skip(size_t);
	bool trim(size_t);
	
	void *append(size_t);
	void *insert(size_t , size_t);
	
	inline const type_traits *typeinfo() const
	{
		return _typeinfo;
	}
	static buffer *create(size_t , const type_traits * = 0);
protected:
	inline buffer() : _typeinfo(0), _size(0), _used(0)
	{ }
	inline ~buffer()
	{ }
#else
MPT_STRUCT(buffer);
MPT_INTERFACE_VPTR(buffer)
{
	MPT_INTERFACE_VPTR(reference) ref;
	MPT_STRUCT(buffer) *(*detach)(MPT_STRUCT(buffer) *, size_t);
	int (*shared)(const MPT_STRUCT(buffer) *);
}; MPT_STRUCT(buffer)
{
	const MPT_INTERFACE_VPTR(buffer) *_vptr;
#endif
	const MPT_STRUCT(type_traits) *_typeinfo;
	size_t _size;
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
	enum { Type = TypeArray };
	
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
	bool set(const reference_wrapper<buffer> &);
	
	int set(value);
	int set(metatype &);
	int printf(const char *fmt, ... );
	
	array & operator=  (const array &);
	array & operator=  (const slice &);
	array & operator+= (const array &);
	array & operator+= (const slice &);
	array & operator=  (struct ::iovec const&);
	array & operator+= (struct ::iovec const&);
protected:
# define _MPT_ARRAY_TYPE(x)     ::mpt::typed_array<x>
# define _MPT_UARRAY_TYPE(x)    ::mpt::unique_array<x>
# define _MPT_REF_ARRAY_TYPE(x) ::mpt::reference_array<x>
	reference_wrapper<content> _buf;
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
	encode_array(DataEncoder = 0);
	~encode_array();
	
	bool prepare(size_t);
	ssize_t push(size_t , const void *);
	bool set_encoding(DataEncoder);
	bool shift(size_t = 0);
	
	bool push(const struct message &);
	
	span<uint8_t> data() const;
protected:
#else
# define MPT_ENCODE_ARRAY_INIT { 0, MPT_ENCODE_INIT, MPT_ARRAY_INIT }
#endif
	MPT_TYPE(DataEncoder)_enc;
	MPT_STRUCT(encode_state) _state;
	MPT_STRUCT(array) _d;
};


#ifdef __cplusplus
template <> inline __MPT_CONST_TYPE int typeinfo<array>::id() {
	return array::Type;
}

/*! reference to buffer segment */
struct slice : array
{
	slice(const array &);
	slice(const slice &);
	slice(array::content * = 0);
	~slice();
	
	span<uint8_t> data() const;
	
	ssize_t write(size_t , const void *, size_t);
	int set(metatype &);
	
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
	long i;
	
	for (i = 0; i < pts; i++) {
		dest[i] = src[i];
	}
}
template<> void copy<double, double>(long pts, const double *, double *);
#endif

__MPT_EXTDECL_BEGIN

/* get range of array */
extern void mpt_drange(double  *, int , const double  *, int);
extern void mpt_frange(float   *, int , const float   *, int);
extern void mpt_irange(int32_t *, int , const int32_t *, int);

/* copy/convert operations with leading dimension */
#if !defined(MPT_COPY_ST) && !defined(MPT_COPY_DT)
extern void mpt_copy64(int , const void *, int , void *, int);
extern void mpt_copy32(int , const void *, int , void *, int);

extern void mpt_copy_fd(int , const float  *, int , double *, int);
extern void mpt_copy_df(int , const double *, int , float  *, int);
#endif

/* iterator with buffer data */
extern MPT_INTERFACE(metatype) *mpt_meta_buffer(const MPT_STRUCT(array) *);
/* treat first string segment as metatype vvalue */
extern MPT_INTERFACE(metatype) *mpt_meta_arguments(const MPT_STRUCT(array) *);

/* array manipulation */
extern size_t mpt_array_reduce(MPT_STRUCT(array) *);
extern int mpt_array_clone(MPT_STRUCT(array) *, const MPT_STRUCT(array) *);

/* add data to array */
extern void *mpt_array_append(MPT_STRUCT(array) *, size_t , const void *__MPT_DEFPAR(0));
extern void *mpt_array_insert(MPT_STRUCT(array) *, size_t , size_t);

/* insert data into buffer */
extern void *mpt_buffer_insert(MPT_STRUCT(buffer) *, size_t , size_t);
/* remove data from buffer */
extern ssize_t mpt_buffer_cut(MPT_STRUCT(buffer) *, size_t , size_t);
/* copy buffer content */
extern long mpt_buffer_copy(MPT_STRUCT(buffer) *, const MPT_STRUCT(buffer) *);
/* get data element */
extern void *mpt_buffer_data(const MPT_STRUCT(buffer) *, size_t , size_t);

/* create and return slice data */
extern void *mpt_array_slice(MPT_STRUCT(array) *, size_t , size_t);

/* write data to slice */
extern ssize_t mpt_slice_write(MPT_STRUCT(slice) *, size_t , const void *, size_t);

/* get string elements from slice */
extern int mpt_slice_get(MPT_STRUCT(slice) *, int , void *);
extern int mpt_slice_advance(MPT_STRUCT(slice) *);

/* snprintf to to array */
extern int mpt_printf(MPT_STRUCT(array) *, const char *, ... );
#if !defined(__STRICT_ANSI__) || __STDC_VERSION__ + 0 >= 199900L || defined(__cplusplus)
extern int mpt_vprintf(MPT_STRUCT(array) *, const char *, va_list );
#endif
/* return zero-terminated buffer data */
extern char *mpt_array_string(MPT_STRUCT(array) *);

/* add message to array */
extern ssize_t mpt_array_push(MPT_STRUCT(encode_array) *, size_t len, const void *data);

/* pointer/metatype array */
extern size_t mpt_array_compact(void **, size_t);
extern size_t mpt_array_move(void *, size_t , size_t , size_t);

/* buffer resizing backends */
extern MPT_STRUCT(buffer) *_mpt_buffer_alloc(size_t, const MPT_STRUCT(type_traits) * __MPT_DEFPAR(0));
extern MPT_STRUCT(buffer) *_mpt_buffer_map(size_t);

/* bit operations */
extern int mpt_bitmap_set(uint8_t *, size_t , size_t);
extern int mpt_bitmap_unset(uint8_t *, size_t , size_t);
extern int mpt_bitmap_get(const uint8_t *, size_t , size_t);

__MPT_EXTDECL_END

#ifdef __cplusplus
inline void *array::base() const
{
	content *d = _buf.reference();
	if (!d || d->typeinfo()) {
		return 0;
	}
	return d->data();
}
inline size_t array::length() const
{
	content *d = _buf.reference();
	return (d && !d->typeinfo()) ? d->length() : 0;
}
inline size_t array::left() const
{
	content *d = _buf.reference();
	return (d && !d->typeinfo()) ? d->left() : 0;
}
inline bool array::shared() const
{
	content *d = _buf.reference();
	return d && d->shared();
}
inline const array::content *array::data() const
{
	return _buf.reference();
}
inline void *array::prepend(size_t len, const void *data)
{
	return insert(0, len, data);
}
inline array &array::operator= (slice const& from)
{
	span<uint8_t> d = from.data();
	set(d.size(), d.begin());
	return *this;
}
inline array &array::operator+= (array const& from)
{
	append(from.length(), from.base());
	return *this;
}
inline array &array::operator+= (slice const& from)
{
	span<uint8_t> d = from.data();
	append(d.size(), d.begin());
	return *this;
}
inline slice::slice(array const& a) : array(a), _off(0)
{
	_len = length();
}
inline slice::~slice()
{ }
inline span<uint8_t> slice::data() const
{
	buffer *b = _buf.reference();
	return span<uint8_t>(b ? ((uint8_t *) (b + 1)) + _off : 0, _len);
}
long compact(span<void *>);
long unused(span<void *>);
bool swap(span<void *>, long , long);
long offset(span<void *>, const void *);

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
	uintptr_t addref() __MPT_OVERRIDE
	{
		return 0;
	}
	content<T> *detach(size_t len) __MPT_OVERRIDE
	{
		size_t align = len % sizeof(T);
		buffer *b = buffer::create(len - align, _typeinfo);
		if (b && !b->copy(*this)) {
			b->unref();
			return 0;
		}
		return static_cast<content<T> *>(b);
	}
	int shared() const __MPT_OVERRIDE
	{
		return 0;
	}
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
		size_t used = _used;
		uint8_t *ptr = static_cast<uint8_t *>(buffer::append(set - used));
		if (!ptr) {
			return false;
		}
		if (!_typeinfo || !_typeinfo->init) {
			set -= used;
			for (size_t pos = 0; pos < set; pos += sizeof(T)) {
				new (ptr + pos) T();
			}
		}
		return true;
	}
	T *insert(long pos)
	{
		if (pos < 0) {
			pos += _used / sizeof(T);
			if (pos < 0) {
				return 0;
			}
		}
		void *ptr = buffer::insert(pos * sizeof(T), sizeof(T));
		if (!ptr) {
			return 0;
		}
		if (!_typeinfo || !_typeinfo->init) {
			return new (ptr) T;
		}
		return static_cast<T *>(ptr);
	}
protected:
	inline ~content()
	{ }
};

/*! typed array with standard operations */
template <typename T>
class unique_array
{
public:
	typedef T* iterator;
	
	static const type_traits &typeinfo()
	{
		static type_traits info;
		
		if (info.init) {
			return info;
		}
		info.init = _data_init;
		info.fini = _data_fini;
		info.size = sizeof(T);
		info.type = ::mpt::typeinfo<T>::id();
		return info;
	}
	unique_array(long len = 0)
	{
		if (len) {
			buffer *b = buffer::create(len * sizeof(T), &typeinfo());
			if (b) {
				_ref.set_reference(static_cast<content<T> *>(b));
				return;
			}
		}
		class dummy : public content<T>
		{
		public:
			inline dummy()
			{
				this->_typeinfo = &unique_array::typeinfo();
			}
			uintptr_t addref() __MPT_OVERRIDE
			{
				return 1;
			}
			void unref() __MPT_OVERRIDE
			{ }
			virtual ~dummy()
			{ }
		};
		static dummy _dummy;
		_ref.set_reference(&_dummy);
	}
	inline iterator begin() const
	{
		content<T> *c = _ref.reference();
		return c ? c->begin() : 0;
	}
	inline iterator end() const
	{
		content<T> *c = _ref.reference();
		return c ? c->end() : 0;
	}
	inline unique_array & operator=(const unique_array &a)
	{
		_ref = a._ref;
		return *this;
	}
	inline unique_array & operator=(const reference_wrapper<content<T> > &v)
	{
		this->_ref = v;
		return *this;
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
		content<T> *d = _ref.reference();
		return d->insert(pos);
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
		content<T> *d = _ref.reference();
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
			_ref.set_reference(n);
			return true;
		}
		_ref.set_reference(c);
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
			_ref.set_reference(c);
			return false;
		}
		content<T> *n;
		if ((n = c->detach(len * sizeof(T)))) {
			_ref.set_reference(n);
			return true;
		}
		_ref.set_reference(c);
		return true;
	}
	bool resize(long len)
	{
		if (!reserve(len)) {
			return false;
		}
		content<T> *d;
		if (len >= 0 && (d = _ref.reference())) {
			return d->set_length(len);
		}
		return true;
	}
protected:
	reference_wrapper<content<T> > _ref;
	
	static void _data_init(const type_traits *, void *ptr)
	{
		new (ptr) T;
	}
	static void _data_fini(void *ptr)
	{
		static_cast<T *>(ptr)->~T();
	}
};
template<typename T>
class typeinfo<unique_array<T> >
{
protected:
	typeinfo();
public:
	static inline __MPT_CONST_EXPR int id()
	{
		return array::Type;
	}
};

template <typename T>
class typed_array : public unique_array<T>
{
public:
	static const type_traits &typeinfo()
	{
		static type_traits info;
		
		if (info.init) {
			return info;
		}
		info.init = unique_array<T>::_data_init;
		info.copy = _data_copy;
		info.fini = unique_array<T>::_data_fini;
		info.size = sizeof(T);
		info.type = ::mpt::typeinfo<T>::id();
		return info;
	}
	typed_array(long len = 0)
	{
		if (len) {
			buffer *b = buffer::create(len * sizeof(T), &typeinfo());
			if (b) {
				this->_ref.set_reference(static_cast<content<T> *>(b));
				return;
			}
		}
		class dummy : public content<T>
		{
		public:
			inline dummy()
			{
				this->_typeinfo = &typed_array::typeinfo();
			}
			uintptr_t addref() __MPT_OVERRIDE
			{
				return 1;
			}
			void unref() __MPT_OVERRIDE
			{ }
			virtual ~dummy()
			{ }
		};
		static dummy _dummy;
		this->_ref.set_reference(&_dummy);
	}
	inline typed_array & operator=(const typed_array &a)
	{
		this->_ref = a._ref;
		return *this;
	}
	bool insert(long pos, const T &v)
	{
		T *ptr;
		if (!(ptr = unique_array<T>::insert(pos))) {
			return false;
		}
		*ptr = v;
		return true;
	}
protected:
	static int _data_copy(void *src, int type, void *dst)
	{
		if (type != typeinfo().type) {
			return BadType;
		}
		T *from = static_cast<T *>(src);
		T *to = static_cast<T *>(dst);
		*to = *from;
		return 0;
	}
};
template<typename T>
class typeinfo<typed_array<T> >
{
protected:
	typeinfo();
public:
	static inline __MPT_CONST_EXPR int id()
	{
		return array::Type;
	}
};

/*! typed array with standard operations */
template <typename T>
class item_array : public unique_array<item<T> >
{
public:
	inline item_array(size_t len = 0) : unique_array<item<T> >(len)
	{ }
	item<T> *append(T *t, const char *id, int len = -1)
	{
		item<T> *it;
		long pos = this->length();
		if (!(it = this->insert(pos))) {
			return 0;
		}
		if (!id || it->set_name(id, len)) {
			it->set_reference(t);
			return it;
		}
		this->resize(pos);
		return 0;
	}
	long count() const
	{
		long len = 0;
		for (item<T> *pos = this->begin(), *to = this->end(); pos != to; ++pos) {
			if (pos->reference()) ++len;
		}
		return len;
	}
	bool compact()
	{
		item<T> *space = 0;
		long len = 0;
		for (item<T> *pos = this->begin(), *to = this->end(); pos != to; ++pos) {
			T *c = pos->reference();
			if (!c) {
				if (!space) space = pos;
				continue;
			}
			++len;
			if (!space) continue;
			space->~item<T>();
			memcpy(space, pos, sizeof(*space));
			memset(pos, 0, sizeof(*pos));
			++space;
		}
		if (!space) {
			return false;
		}
		this->_ref.reference()->set_length(len);
		return true;
	}
};
template<typename T>
class typeinfo<item_array<T> >
{
protected:
	typeinfo();
public:
	static inline __MPT_CONST_EXPR int id()
	{
		return array::Type;
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
		this->_ref = static_cast<pointer_array<T> &>(from)._ref;
		return *this;
	}
	inline void compact()
	{
		compact(generic());
	}
	inline long unused() const
	{
		return unused(generic());
	}
	inline long offset(const T *ref) const
	{
		return offset(generic(), ref);
	}
	inline bool swap(long p1, long p2) const
	{
		return swap(generic(), p1, p2);
	}
protected:
	inline span<void *> generic() const
	{
		return span<void *>(this->begin(), this->length());
	}
};
template<typename T>
class typeinfo<pointer_array<T> >
{
protected:
	typeinfo();
public:
	static inline __MPT_CONST_EXPR int id()
	{
		return array::Type;
	}
};

/*! extendable array for reference types */
template <typename T>
class reference_array : public unique_array<reference_wrapper<T> >
{
public:
	typedef reference_wrapper<T>* iterator;
	
	inline reference_array(size_t len = 0) : unique_array<reference_wrapper<T> >(len)
	{ }
	bool insert(long pos, T *ref)
	{
		reference_wrapper<T> *ptr = unique_array<reference_wrapper<T> >::insert(pos);
		if (!ptr) {
			return false;
		}
		ptr->set_reference(ref);
		return true;
	}
	bool set(long pos, T *ref)
	{
		reference_wrapper<T> *ptr = unique_array<reference_wrapper<T> >::get(pos);
		if (!ptr) {
			return false;
		}
		ptr->set_reference(ref);
		return true;
	}
	long clear(const T *ref = 0) const
	{
		reference_wrapper<T> *ptr = this->begin();
		long elem = 0;
		
		for (long i = 0, len = this->length(); i < len; ++i) {
			T *match = ptr[i].reference();
			if (!match || (ref && (match != ref))) {
				continue;
			}
			ptr[i].set_reference(0);
			++elem;
		}
		return elem;
	}
	long count() const
	{
		long len = 0;
		for (reference_wrapper<T> *pos =  this->begin(), *to =  this->end(); pos != to; ++pos) {
			if (pos->reference()) ++len;
		}
		return len;
	}
	void compact()
	{
		::mpt::compact(span<void *>(reinterpret_cast<void **>(this->begin()), this->length()));
	}
};
template<typename T>
class typeinfo<reference_array<T> >
{
protected:
	typeinfo();
public:
	static inline __MPT_CONST_EXPR int id()
	{
		return array::Type;
	}
};

class message_store : public logger, public reference_wrapper<metatype>
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
template<> inline __MPT_CONST_TYPE int typeinfo<message_store::entry>::id()
{
	return typeinfo<array>::id();
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
class typeinfo<map<K, V> >
{
protected:
	typeinfo();
public:
	static inline __MPT_CONST_EXPR int id()
	{
		return array::Type;
	}
};

#endif /* __cplusplus */

__MPT_NAMESPACE_END

#endif /* _MPT_ARRAY_H */
