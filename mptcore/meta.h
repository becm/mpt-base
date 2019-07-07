/*!
 * MPT core library
 *  basic types and definitions
 */

#ifndef _MPT_META_H
#define _MPT_META_H  @INTERFACE_VERSION@

#include "core.h"

#ifndef __cplusplus
# define MPT_metatype_convert(m, t, d) ((m)->_vptr->convertable.convert((MPT_INTERFACE(convertable) *) (m), t, d))
#endif

__MPT_NAMESPACE_BEGIN

/*! generic metatype interface */
#ifdef __cplusplus
MPT_INTERFACE(metatype) : public convertable
{
protected:
	inline ~metatype() {}
public:
	enum { Type = _TypeMetaBase };
	
	class basic;
	
	static metatype *create(value);
	static metatype *create(int, const void *);
	
	int convert(int , void *) __MPT_OVERRIDE;
	
	virtual void unref() = 0;
	virtual uintptr_t addref();
	virtual metatype *clone() const = 0;
};
template <> inline __MPT_CONST_TYPE int typeinfo<metatype>::id()
{
	return metatype::Type;
}

inline uintptr_t metatype::addref()
{
	return 0;
}
#else
MPT_INTERFACE(metatype);
MPT_INTERFACE_VPTR(metatype)
{
	MPT_INTERFACE_VPTR(convertable) convertable;
	void (*unref)(MPT_INTERFACE(metatype) *);
	uintptr_t (*addref)(MPT_INTERFACE(metatype) *);
	MPT_INTERFACE(metatype) *(*clone)(const MPT_INTERFACE(metatype) *);
}; MPT_INTERFACE(metatype) {
	const MPT_INTERFACE_VPTR(metatype) *_vptr;
};
#endif

/*! generic iterator interface */
#ifdef __cplusplus
MPT_INTERFACE(iterator)
{
protected:
	inline ~iterator() {}
public:
	enum { Type = TypeIterator };
	
	virtual int get(int, void *) = 0;
	virtual int advance();
	virtual int reset();
	
	template <typename T>
	bool consume(T &val)
	{
		T tmp;
		if (get(typeinfo<T>::id(), &tmp) <= 0) {
			return false;
		}
		if (advance() < 0) {
			return false;
		}
		val = tmp;
		return true;
	}
};
template <> inline __MPT_CONST_TYPE int typeinfo<iterator>::id()
{
	return iterator::Type;
}
#else
MPT_INTERFACE(iterator);
MPT_INTERFACE_VPTR(iterator)
{
	int (*get)(MPT_INTERFACE(iterator) *, int , void *);
	int (*advance)(MPT_INTERFACE(iterator) *);
	int (*reset)(MPT_INTERFACE(iterator) *);
}; MPT_INTERFACE(iterator) {
	const MPT_INTERFACE_VPTR(iterator) *_vptr;
};
#endif

/*! generic consumable entity */
MPT_STRUCT(consumable)
{
#ifdef __cplusplus
	inline consumable(convertable &val)
	{
		if ((_it = val.cast<iterator>())) {
			return;
		}
		val.convert(_val.Type, &_val);
	}
protected:
#else
# define MPT_CONSUMABLE_INIT { 0, MPT_VALUE_INIT }
#endif
	MPT_INTERFACE(iterator) *_it;
	MPT_STRUCT(value) _val;
};

#ifdef __cplusplus
inline metatype *metatype::clone() const
{
	return 0;
}
inline int iterator::advance()
{
	return 0;
}
inline int iterator::reset()
{
	return 0;
}

/* basic metatype to support typeinfo */
class metatype::basic : public metatype
{
protected:
	inline ~basic()
	{ }
public:
	basic(size_t post);
	
	int convert(int , void *) __MPT_OVERRIDE;
	
	void unref() __MPT_OVERRIDE;
	basic *clone() const __MPT_OVERRIDE;
	
	bool set(const char *, int);
	
	static basic *create(const char *, int);
};
/* generic implementation for metatype */
template <typename T>
class meta_value : public metatype
{
public:
	inline meta_value(const T *val = 0) : _val(val ? *val : 0)
	{ }
	inline meta_value(const T &val) : _val(val)
	{ }
	virtual ~meta_value()
	{ }
	void unref() __MPT_OVERRIDE
	{
		delete this;
	}
	int convert(int type, void *dest) __MPT_OVERRIDE
	{
		static const int me = typeinfo<T>::id();
		if (!type) {
			if (dest) {
				*static_cast<const uint8_t **>(dest) = 0;
			}
			return me;
		}
		if (type != me) {
			return BadType;
		}
		if (dest) {
			*static_cast<T *>(dest) = _val;
		}
		return me;
	}
	meta_value *clone() const __MPT_OVERRIDE
	{
		return new meta_value(_val);
	}
protected:
	T _val;
};
template <typename T>
class typeinfo<meta_value<T> >
{
protected:
	typeinfo();
public:
	static int id()
	{
		return metatype::Type;
	}
};

template <typename T>
class source : public iterator
{
public:
	source(const T *val, long len = 1) : _d(val, len), _pos(0)
	{ }
	virtual ~source()
	{ }
	int get(int type, void *dest) __MPT_OVERRIDE
	{
		int fmt;
		if ((fmt = this->content()) < 0) {
			return BadType;
		}
		const T *val = _d.nth(_pos);
		if (!val) {
			return MissingData;
		}
		type = convert((const void **) &val, fmt, dest, type);
		if (type < 0) {
			return type;
		}
		return fmt;
	}
	int advance() __MPT_OVERRIDE
	{
		int pos = _pos + 1;
		if (pos > _d.length()) {
			return MissingData;
		}
		if (pos == _d.length()) {
			return 0;
		}
		_pos = pos;
		return content();
	}
	int reset() __MPT_OVERRIDE
	{
		_pos = 0;
		return _d.size();
	}
	inline static int content()
	{
		return typeinfo<T>::id();
	}
protected:
	span<const T> _d;
	int _pos;
};
template <typename T>
class typeinfo<source<T> >
{
protected:
	typeinfo();
public:
	static int id()
	{
		return iterator::Type;
	}
};
#endif

__MPT_EXTDECL_BEGIN


/* create meta type element */
extern MPT_INTERFACE(metatype) *mpt_meta_new(MPT_STRUCT(value));
/* set (zero-terminated string) node data */
extern int mpt_meta_set(MPT_INTERFACE(metatype) **, const MPT_STRUCT(value) *);

/* creat basic text small metatype */
extern MPT_INTERFACE(metatype) *mpt_meta_geninfo(size_t);

/* create meta type element */
extern MPT_INTERFACE(metatype) *mpt_metatype_default();

/* initialize geninfo data */
extern int _mpt_geninfo_size(size_t);
extern int _mpt_geninfo_init(void *, size_t);
/* operations on geninfo data */
extern int _mpt_geninfo_set(void *, const char *, int __MPT_DEFPAR(-1));
extern int _mpt_geninfo_flags(const void *, int);
extern int _mpt_geninfo_conv(const void *, int , void *);
/* clone geninfo content */
extern MPT_INTERFACE(metatype) *_mpt_geninfo_clone(const void *);

/* assign to value via iterator */
extern int mpt_process_value(MPT_STRUCT(value) *, int (*)(void *, MPT_INTERFACE(iterator) *), void *);
extern int mpt_process_vararg(const char *, va_list, int (*)(void *, MPT_INTERFACE(iterator) *), void *);
extern MPT_INTERFACE(metatype) *mpt_iterator_value(MPT_STRUCT(value), int __MPT_DEFPAR(-1));
extern MPT_INTERFACE(metatype) *mpt_iterator_string(const char *, const char *__MPT_DEFPAR(0));

/* get value and advance source */
extern int mpt_consumable_setup(MPT_STRUCT(consumable) *, MPT_INTERFACE(convertable) *);
extern int mpt_consume_double(MPT_STRUCT(consumable) *, double *);
extern int mpt_consume_uint(MPT_STRUCT(consumable) *, uint32_t *);
extern int mpt_consume_int(MPT_STRUCT(consumable) *, int32_t *);
extern int mpt_consume_key(MPT_STRUCT(consumable) *, const char **);


__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_META_H */
