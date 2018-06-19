/*!
 * MPT core library
 *  basic types and definitions
 */

#ifndef _MPT_META_H
#define _MPT_META_H  @INTERFACE_VERSION@

#include "core.h"

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(property);

/*! generic metatype interface */
#ifdef __cplusplus
MPT_INTERFACE(metatype) : public reference
{
protected:
	inline ~metatype() {}
public:
	enum { Type = TypeMeta };
	
	class Basic;
	
	const char *string() const;
	void *pointer(int) const;
	
	inline const uint8_t *types() const
	{
		uint8_t *t = 0;
		return (conv(0, &t) < 0) ? 0 : t;
	}
	template <typename T>
	inline T *cast() const
	{
		void *ptr = 0;
		if (conv(typeinfo<T *>::id(), &ptr) < 0) {
			return 0;
		}
		return static_cast<T *>(ptr);
	}
	inline operator const char *() const
	{
		return string();
	}
	
	static metatype *create(value);
	static metatype *create(int, const void *);
	
	virtual int conv(int , void *) const = 0;
	virtual metatype *clone() const;
	
	inline int type() const
	{ return conv(0, 0); }
};
template <> inline __MPT_CONST_TYPE int typeinfo<metatype *>::id()
{
	return metatype::Type;
}
template <> inline __MPT_CONST_TYPE int typeinfo<reference_wrapper <metatype> >::id()
{
	return typeinfo<metatype *>::id();
}
#else
MPT_INTERFACE(metatype);
MPT_INTERFACE_VPTR(metatype)
{
	MPT_INTERFACE_VPTR(reference) ref;
	int (*conv)(const MPT_INTERFACE(metatype) *, int , void *);
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
};
template <> inline __MPT_CONST_TYPE int typeinfo<iterator *>::id()
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
	inline consumable(const metatype &mt)
	{
	    if ((_it = mt.cast<iterator>())) {
		    return;
	    }
	    mt.conv(_val.Type, &_val);
	}
	template <typename T>
	bool consume(T &val)
	{
		int type = typeinfo<T>::id();
		if (_it) {
			T tmp;
			if (_it->get(type, &tmp) <= 0) {
				return false;
			}
			if (_it->advance() < 0) {
				return false;
			}
			val = tmp;
			return true;
		}
		if (_val.fmt) {
			const T *ptr;
			if (!*_val.fmt
			    || type != *_val.fmt
			    || !(ptr = static_cast<const T *>(_val.ptr))) {
				return false;
			}
			val = *ptr;
			++_val.fmt;
			_val.ptr = ptr + 1;
			return true;
		}
		if (type == 's') {
			val = static_cast<const char *>(_val.ptr);
			_val.ptr = 0;
			return true;
		}
		return BadType;
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

/* specialize metatype string cast */
template <> inline const char *metatype::cast<const char>() const
{
	return string();
}

/* basic metatype to support typeinfo */
class metatype::Basic : public metatype
{
protected:
	inline ~Basic() {}
public:
	Basic(size_t post);
	
	void unref() __MPT_OVERRIDE;
	int conv(int , void *) const __MPT_OVERRIDE;
	Basic *clone() const __MPT_OVERRIDE;
	
	bool set(const char *, int);
	
	static Basic *create(const char *, int);
};
/* generic implementation for metatype */
template <typename T>
class Metatype : public metatype
{
public:
	inline Metatype(const T *val = 0) : _val(val ? *val : 0)
	{ }
	inline Metatype(const T &val) : _val(val)
	{ }
	virtual ~Metatype()
	{ }
	void unref()
	{
		delete this;
	}
	int conv(int type, void *dest) const
	{
		static const int me = typeinfo<T>::id();
		if (!type) {
			if (dest) {
				*static_cast<const char **>(dest) = 0;
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
	metatype *clone() const
	{
		return new Metatype(_val);
	}
protected:
	T _val;
};
template <typename T>
class typeinfo<Metatype<T> >
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
class Source : public iterator
{
public:
	Source(const T *val, long len = 1) : _d(val, len), _pos(0)
	{ }
	virtual ~Source()
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
class typeinfo<Source<T> >
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


/* log metatype info */
extern int mpt_meta_info(const MPT_INTERFACE(metatype) *, MPT_STRUCT(property) *);

/* try to log to metatype instance */
extern int mpt_meta_vlog(const MPT_INTERFACE(metatype) *, const char *, int , const char *, va_list);
extern int mpt_meta_log(const MPT_INTERFACE(metatype) *, const char *, int , const char *, ... );


/* create meta type element */
extern MPT_INTERFACE(metatype) *mpt_meta_new(MPT_STRUCT(value));
/* set (zero-terminated string) node data */
extern int mpt_meta_set(MPT_INTERFACE(metatype) **, const MPT_STRUCT(value) *);

/* creat basic text small metatype */
extern MPT_INTERFACE(metatype) *mpt_meta_geninfo(size_t);

/* create meta type element */
extern MPT_INTERFACE(metatype) *mpt_metatype_default();

/* get node/metatype text/raw data */
extern const char *mpt_meta_data(const MPT_INTERFACE(metatype) *, size_t *__MPT_DEFPAR(0));
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
extern int mpt_consumable_setup(MPT_STRUCT(consumable) *, const MPT_INTERFACE(metatype) *);
extern int mpt_consume_double(MPT_STRUCT(consumable) *, double *);
extern int mpt_consume_uint(MPT_STRUCT(consumable) *, uint32_t *);
extern int mpt_consume_int(MPT_STRUCT(consumable) *, int32_t *);
extern int mpt_consume_key(MPT_STRUCT(consumable) *, const char **);


__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_META_H */
