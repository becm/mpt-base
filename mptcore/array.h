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
struct iovec;
#endif

#include "core.h"

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(slice);

MPT_INTERFACE(metatype);

/*! header for data segment */
#ifdef __cplusplus
MPT_STRUCT(buffer) : public unrefable
{
public:
	enum { Type = TypeBuffer };
	
	void unref() __MPT_OVERRIDE;
	virtual buffer *detach(long capacity = -1) = 0;
	virtual int content() const = 0;
	
	uintptr_t addref();
	bool shared() const;
	size_t left() const;
protected:
	buffer(size_t);
	inline ~buffer()
	{ }
#else
MPT_STRUCT(buffer);
MPT_INTERFACE_VPTR(buffer)
{
	MPT_INTERFACE_VPTR(unrefable) ref;
	MPT_STRUCT(buffer) *(*detach)(MPT_STRUCT(buffer) *, long);
	int (*content)(const MPT_STRUCT(buffer) *);
}; MPT_STRUCT(buffer)
{
	const MPT_INTERFACE_VPTR(buffer) *_vptr;
#endif
	MPT_STRUCT(refcount) _ref;
	uintptr_t _size;
	uintptr_t _used;
};

/*! reference to buffer data */
MPT_STRUCT(array)
{
#ifdef __cplusplus
	class Data : public buffer
	{
	public:
		inline Data(size_t post) : buffer(post)
		{ }
		
		Data *detach(long capacity = -1) __MPT_OVERRIDE;
		int content() const __MPT_OVERRIDE;
		
		inline size_t length() const
		{ return _used; }
		inline size_t left() const
		{ return _size - _used; }
		inline void *data() const
		{ return static_cast<void *>(const_cast<Data *>(this) + 1); }
		
		bool setLength(size_t);
		static Data *create(size_t);
	protected:
		inline ~Data()
		{ }
	};
	enum { Type = TypeArray };
	
	array(array const&);
	array(size_t = 0);
	
	const Data *data() const;
	
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
	bool set(const Reference<buffer> &);
	
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
# define _MPT_ARRAY_TYPE(x)     ::mpt::Array<x>
# define _MPT_UARRAY_TYPE(x)    ::mpt::UniqueArray<x>
# define _MPT_REF_ARRAY_TYPE(x) ::mpt::RefArray<x>
	Reference<Data> _buf;
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
	bool setEncoding(DataEncoder);
	bool shift(size_t = 0);
	
	bool push(const struct message &);
	
	Slice<uint8_t> data() const;
protected:
#else
# define MPT_ENCODE_ARRAY_INIT { 0, MPT_ENCODE_INIT, MPT_ARRAY_INIT }
#endif
	MPT_TYPE(DataEncoder)_enc;
	MPT_STRUCT(encode_state) _state;
	MPT_STRUCT(array) _d;
};


#ifdef __cplusplus
/*! reference to buffer segment */
struct slice : array
{
	slice(const array &);
	slice(const slice &);
	slice(Data * = 0);
	~slice();
	
	Slice<uint8_t> data() const;
	
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
	
	if (!ld) return;
	
	while (--len > 0) {
		val += ld;
		if (*val < range[0]) range[0] = *val;
		if (*val > range[1]) range[1] = *val;
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
	intptr_t i, j;
	
	if (pts <= 0) return;
	
	if (!ldd) {
		dest[0] = src[(pts-1) * lds];
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

/* array manipulation */
extern size_t mpt_array_reduce(MPT_STRUCT(array) *);
extern void mpt_array_clone(MPT_STRUCT(array) *, const MPT_STRUCT(array) *);

/* add data to array */
extern void *mpt_array_append(MPT_STRUCT(array) *, size_t , const void *__MPT_DEFPAR(0));
extern void *mpt_array_insert(MPT_STRUCT(array) *, size_t , size_t);

/* insert data into buffer */
extern void *mpt_buffer_insert(MPT_STRUCT(buffer) *, size_t , size_t);
/* remove data from buffer */
extern ssize_t mpt_buffer_cut(MPT_STRUCT(buffer) *, size_t , size_t);
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
extern MPT_STRUCT(buffer) *_mpt_buffer_alloc(size_t);
extern MPT_STRUCT(buffer) *_mpt_buffer_map  (size_t);

/* bit operations */
extern int mpt_bitmap_set(uint8_t *, size_t , size_t);
extern int mpt_bitmap_unset(uint8_t *, size_t , size_t);
extern int mpt_bitmap_get(const uint8_t *, size_t , size_t);

__MPT_EXTDECL_END

#ifdef __cplusplus
inline bool buffer::shared() const
{
	return _ref.value() > 1;
}
inline void *array::base() const
{
	Data *d = _buf.pointer();
	if (!d || d->content()) {
		return 0;
	}
	return d->data();
}
inline size_t array::length() const
{
	Data *d = _buf.pointer();
	return d && !d->content() ? d->length() : 0;
}
inline size_t array::left() const
{
	Data *d = _buf.pointer();
	return d && !d->content() ? d->left() : 0;
}
inline bool array::shared() const
{
	buffer *b = _buf.pointer();
	return b && b->shared();
}
inline const array::Data *array::data() const
{
	return _buf.pointer();
}
inline void *array::prepend(size_t len, const void *data)
{ return insert(0, len, data); }

inline array &array::operator= (slice const& from)
{ Slice<uint8_t> d = from.data(); set(d.length(), d.base()); return *this; }
inline array &array::operator+= (array const& from)
{ append(from.length(), from.base()); return *this; }
inline array &array::operator+= (slice const& from)
{ Slice<uint8_t> d = from.data(); append(d.length(), d.base()); return *this; }

template<> inline __MPT_CONST_EXPR int typeIdentifier<array>() { return array::Type; }

inline slice::slice(array const& a) : array(a), _off(0)
{ _len = length(); }
inline slice::~slice()
{ }
inline Slice<uint8_t> slice::data() const
{
	buffer *b = _buf.pointer();
	return Slice<uint8_t>(b ? ((uint8_t *) (b + 1)) + _off : 0, _len);
}
long compact(Slice<void *>);
long unused(Slice<void *>);
bool swap(Slice<void *>, long , long);
long offset(Slice<void *>, const void *);

template <typename T>
void move(T *v, size_t from, size_t to)
{
	if (from == to) return;
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
class Content : public buffer
{
public:
	void unref() __MPT_OVERRIDE
	{
		if (!_ref.lower()) {
			T *t = data();
			for (long i = 0, end = length(); i < end; ++i) t[i].~T();
			std::free(this);
		}
	}
	inline T *data() const
	{
		return static_cast<T *>(const_cast<void *>(static_cast<const void *>(this + 1)));
	}
	inline long size() const
	{
		return _size / sizeof(T);
	}
	inline long length() const
	{
		return _used / sizeof(T);
	}
	bool setLength(long len)
	{
		long end = length();
		if (len < 0 && (len += end) < 0) {
			return false;
		}
		long max = size();
		if (len > max) {
			return false;
		}
		T *t = data();
		for (long i = end; i < len; ++i) {
			new (t + i) T();
		}
		for (long i = len; i < end; ++i) {
			t[i].~T();
		}
		_used = len * sizeof(T);
		return true;
	}
	Content *detach(long len = -1) __MPT_OVERRIDE
	{
		/* detach requires new instance */
		if (shared()) {
			return 0;
		}
		/* keep current size */
		if (len < 0) {
			return this;
		}
		long max = length();
		/* no expand operation */
		if (len > max) {
			return 0;
		}
		/* shrink to requested size */
		T *t = data();
		for (long i = len; i < max; ++i) t[i].~T();
		_used = len * sizeof(*t);
		return this;
	}
	inline int content() const __MPT_OVERRIDE
	{
		return typeIdentifier<T>();
	}
	static Content *create(long len = -1)
	{
		if (len < 1) len = 4;
		len *= sizeof(T);
		void *d = std::malloc(sizeof(Content) + len);
		return new (d) Content(len);
	}
	T *insert(long pos)
	{
		long end = length();
		if (pos < 0 && (pos += end) < 0) {
			return 0;
		}
		long max = size();
		if (pos >= max || end >= max) {
			return 0;
		}
		T *t = data();
		if ((max = end - pos) > 0) {
			/* move elements after position */
			std::memmove(t + pos + 1, t + pos, max * sizeof(*t));
			_used = (end + 1) * sizeof(*t);
		} else {
			/* init created filler elements */
			for (long i = end; i < pos; ++i) {
				new (t + i) T();
			}
			_used = (pos + 1) * sizeof(*t);
		}
		return new (t + pos) T;
	}
protected:
	inline Content(size_t len) : buffer(len)
	{ }
	inline ~Content()
	{ }
};

/*! typed array with standard operations */
template <typename T>
class UniqueArray
{
public:
	typedef T* iterator;
	
	class Data : public Content<T>
	{
	public:
		Content<T> *detach(long len = -1) __MPT_OVERRIDE
		{
			/* detach requires new instance */
			if (this->shared()) {
				return 0;
			}
			/* shrink to requested size */
			if (len < this->size()) {
				return Content<T>::detach(len);
			}
			len *= sizeof(T);
			/* increase reserved size */
			Data *d = static_cast<Data *>(std::realloc(this, sizeof(*this) + len));
			if (d) {
				d->_size = len;
			}
			return d;
		}
		static Data *create(long len = -1)
		{
			if (len < 1) len = 4;
			len *= sizeof(T);
			void *d = std::malloc(sizeof(Data) + len);
			return new (d) Data(len);
		}
	protected:
		inline Data(long len) : Content<T>(len)
		{ }
		inline ~Data()
		{ }
	};
	inline UniqueArray(long len = 0)
	{
		if (len) _ref.setPointer(Data::create(len));
	}
	inline iterator begin() const
	{
		Content<T> *d = _ref.pointer();
		return d ? d->data() : 0;
	}
	inline iterator end() const
	{
		Content<T> *d = _ref.pointer();
		return d ? d->data() + d->length() : 0;
	}
	UniqueArray & operator=(const UniqueArray &a)
	{
		_ref = a._ref;
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
		long min = length();
		if (pos < 0 && (pos += min) < 0) {
			return 0;
		}
		if (pos > min) {
			min = pos;
		}
		if (!reserve(min + 1)) {
			return 0;
		}
		Content<T> *d = _ref.pointer();
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
		Content<T> *d = _ref.pointer();
		return d ? d->length() : 0;
	}
	inline Slice<const T> slice() const
	{
		return Slice<const T>(begin(), length());
	}
	bool detach()
	{
		Content<T> *n, *d = _ref.detach();
		if ((n = d->detach())) {
			_ref.setPointer(n);
			return true;
		} else {
			_ref.setPointer(d);
			return false;
		}
	}
	bool reserve(long len)
	{
		Content<T> *d;
		if (!(d = _ref.detach())) {
			if (len <= 0) {
				return true;
			}
			if (!(d = Data::create(len))) {
				return false;
			}
			_ref.setPointer(d);
			return true;
		}
		if (!len && d->shared()) {
			d->unref();
			return true;
		}
		Content<T> *r;
		if ((r = d->detach(len))) {
			_ref.setPointer(r);
			return true;
		} else {
			_ref.setPointer(d);
			return false;
		}
	}
	bool resize(long len)
	{
		if (!reserve(len)) {
			return false;
		}
		Content<T> *d;
		if (len >= 0 && (d = _ref.pointer())) {
			return d->setLength(len);
		}
		return true;
	}
protected:
	Reference<Content<T> > _ref;
};

template <typename T>
class Array : public UniqueArray<T>
{
public:
	class Data : public UniqueArray<T>::Data
	{
	public:
		Content<T> *detach(long len) __MPT_OVERRIDE
		{
			/* keep current size */
			if (len < 0) {
				len = this->length();
			}
			/* force new instance */
			if (this->shared()) {
				Data *d = create(len);
				/* clone existing data */
				T *o = this->data();
				T *t = d->data();
				long elem = this->length();
				for (long i = 0; i < elem; ++i) {
					t[i] = o[i];
				}
				d->_used = elem * sizeof(*t);
				this->unref();
				return d;
			}
			return UniqueArray<T>::Data::detach(len);
		}
		static Data *create(long len) {
			if (len < 1) len = 4;
			len *= sizeof(T);
			void *d = std::malloc(sizeof(Data) + len);
			return new (d) Data(len);
		}
	protected:
		inline Data(long len) : UniqueArray<T>::Data(len)
		{ }
		inline ~Data()
		{ }
	};
	inline Array(long len = 0)
	{
		if (len) {
			this->_ref.setPointer(Data::create(len));
		}
	}
	inline Array & operator=(const Reference<Content<T *> > &v)
	{
		this->_ref = v;
		return *this;
	}
	bool insert(long pos, const T &v)
	{
		T *ptr;
		if (!(ptr = UniqueArray<T>::insert(pos))) {
			return false;
		}
		*ptr = v;
		return true;
	}
};

/*! typed array with standard operations */
template <typename T>
class ItemArray : public UniqueArray<Item<T> >
{
public:
	inline ItemArray(size_t len = 0) : UniqueArray<Item<T> >(len)
	{ }
	Item<T> *append(T *t, const char *id, int len = -1)
	{
		Item<T> *it;
		long pos = this->length();
		if (!(it = this->insert(pos))) {
			return 0;
		}
		if (it->setName(id, len)) {
			it->setPointer(t);
			return it;
		}
		this->resize(pos);
		return 0;
	}
	bool compact()
	{
		Item<T> *space = 0;
		size_t len = 0;
		for (Item<T> *pos =  this->begin(), *to =  this->end(); pos != to; ++pos) {
			T *c = pos->pointer();
			if (!c) {
				if (!space) space = pos;
				continue;
			}
			++len;
			if (!space) continue;
			memcpy(space, pos, sizeof(*space));
			memset(pos, 0, sizeof(*pos));
			do {
				++space;
			} while ((c = space->pointer()) && space < pos);
		}
		if (!space) return false;
		this->_ref.pointer()->setLength(len);
		return true;
	}
};

/*! basic pointer array */
template <typename T>
class PointerArray : public Array<T *>
{
public:
	typedef T** iterator;
	
	inline PointerArray(long len = 0) : Array<T *>(len)
	{ }
	inline PointerArray &operator =(Array<T *> &from)
	{
		this->_ref = static_cast<PointerArray<T> &>(from)._ref;
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
	inline Slice<void *> generic() const
	{
		return Slice<void *>(this->begin(), this->length());
	}
};

/*! extendable array for reference types */
template <typename T>
class RefArray : public UniqueArray<Reference<T> >
{
public:
	typedef Reference<T>* iterator;
	
	inline RefArray(size_t len = 0) : UniqueArray<Reference<T> >(len)
	{ }
	bool insert(long pos, T *ref)
	{
		Reference<T> *ptr = UniqueArray<Reference<T> >::insert(pos);
		if (!ptr) {
			return false;
		}
		ptr->setPointer(ref);
		return true;
	}
	bool set(long pos, T *ref)
	{
		Reference<T> *ptr = UniqueArray<Reference<T> >::get(pos);
		if (!ptr) {
			return false;
		}
		ptr->setPointer(ref);
		return true;
	}
	long clear(const T *ref = 0) const
	{
		Reference<T> *ptr = this->begin();
		long elem = 0, len = this->length();
		if (!ref) {
			for (long i = 0; i < len; ++i) {
				T *match;
				if (!(match = ptr[i].pointer())) continue;
				ptr[i].setPointer(0);
				++elem;
			}
			return elem;
		}
		for (long i = 0; i < len; ++i) {
			T *match;
			if (!(match = ptr[i].pointer()) || (match != ref)) {
				continue;
			}
			ptr[i].setPointer(0);
			++elem;
		}
		return elem;
	}
	void compact()
	{
		::mpt::compact(Slice<void *>(reinterpret_cast<void **>(this->begin()), this->length()));
	}
};

class LogStore : public logger, public Reference<logger>
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
	LogStore(logger * = logger::defaultInstance());
	virtual ~LogStore();
	
	class Entry;
	
	void unref() __MPT_OVERRIDE;
	int log(const char *, int, const char *, va_list) __MPT_OVERRIDE;
	
	virtual const Entry *nextEntry();
	virtual void clearLog();
	
	virtual bool setIgnoreLevel(int);
	virtual bool setFlowFlags(int);
	
	inline int flowFlags() const
	{ return _flags; }
	
	inline const Slice<const Entry> logEnries() const
	{ return _msg.slice(); }
protected:
	Array<Entry> _msg;
	uint32_t _act;
	uint8_t  _flags;
	uint8_t  _ignore;
	uint8_t  _level;
};
/*! storage for log messages */
class LogStore::Entry : array
{
public:
	int type() const;
	const char *source() const;
	Slice<const char> data(int part = 0) const;
	int set(const char *, int, const char *, va_list);
protected:
	struct Header
	{
		uint8_t from;
		uint8_t args;
		uint8_t _cmd;
		uint8_t type;
	};
};
template<> inline __MPT_CONST_EXPR int typeIdentifier<LogStore::Entry>() {
	return typeIdentifier<array>();
}

/*! linear search map type */
template <typename K, typename V>
class Map
{
public:
	class Element
	{
	public:
		inline Element(const K &k = K(), const V &v = V()) : key(k), value(v)
		{ }
		K key;
		V value;
	};
	typedef const Element * const_iterator;
	
	inline const_iterator begin() const
	{ return _d.begin(); }
	inline const_iterator end() const
	{ return _d.end(); }
	
	bool set(const K &key, const V &value)
	{
		V *d = get(key);
		if (!d) return _d.insert(_d.length(), Element(key, value));
		*d = value;
		return true;
	}
	bool append(const K &key, const V &value)
	{
		return _d.insert(_d.length(), Element(key, value));
	}
	V *get(const K &key) const
	{
		for (Element *c = _d.begin(), *e = _d.end(); c < e; ++c) {
			if (c->key == key) {
				return &e->value;
			}
		}
		return 0;
	}
	inline Array<V> values(const K &key) const
	{
		return values(&key);
	}
	Array<V> values(const K *key = 0) const
	{
		Array<V> a;
		for (Element *c = _d.begin(), *e = _d.end(); c < e; ++c) {
			if (!key || c->key == *key) {
				a.insert(a.length(), c->value);
			}
		}
		return a;
	}
protected:
	Array<Element> _d;
};

#endif /* __cplusplus */

__MPT_NAMESPACE_END

#endif /* _MPT_ARRAY_H */
