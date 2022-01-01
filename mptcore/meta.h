/*!
 * MPT core library
 *  basic types and definitions
 */

#ifndef _MPT_META_H
#define _MPT_META_H  @INTERFACE_VERSION@

#ifdef __cplusplus
# include "types.h"
#else
# include "core.h"
#endif

#ifndef __cplusplus
# define MPT_metatype_convert(m, t, d) ((m)->_vptr->convertable.convert((MPT_INTERFACE(convertable) *) (m), t, d))
#endif

__MPT_NAMESPACE_BEGIN

/*! generic metatype interface */
#ifdef __cplusplus
MPT_INTERFACE(metatype) : public convertable
{
protected:
	inline ~metatype()
	{ }
public:
	class basic_instance;
	class generic_instance;
	
	static metatype *create(const value &);
	
	int convert(int , void *) __MPT_OVERRIDE;
	
	virtual void unref() = 0;
	virtual uintptr_t addref();
	virtual metatype *clone() const = 0;
};
template <> inline __MPT_CONST_TYPE int type_properties<metatype *>::id(bool) {
	return TypeMetaPtr;
}
template <> inline const struct type_traits *type_properties<metatype *>::traits() {
	return type_traits::get(id(true));
}

template <> inline __MPT_CONST_TYPE int type_properties<reference<metatype> >::id(bool) {
	return TypeMetaRef;
}
template <> inline const struct type_traits *type_properties<reference<metatype> >::traits() {
	return type_traits::get(id(true));
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
	virtual const struct value *value() = 0;
	virtual int advance();
	virtual int reset();
	
	template <typename T>
	bool get(T &val)
	{
		const struct value *src = value();
		if (!src) {
			return false;
		}
		int type = type_properties<T>::id(true);
		if (type <= 0) {
			return false;
		}
		return src->convert(type, &val) >= 0;
	}
};
template <> inline __MPT_CONST_TYPE int type_properties<iterator *>::id(bool) {
	return TypeIteratorPtr;
}
template <> inline const struct type_traits *type_properties<iterator *>::traits() {
	return type_traits::get(id(true));
}
#else
MPT_INTERFACE(iterator);
MPT_INTERFACE_VPTR(iterator)
{
	const MPT_STRUCT(value) *(*value)(MPT_INTERFACE(iterator) *);
	int (*advance)(MPT_INTERFACE(iterator) *);
	int (*reset)(MPT_INTERFACE(iterator) *);
}; MPT_INTERFACE(iterator) {
	const MPT_INTERFACE_VPTR(iterator) *_vptr;
};
#endif

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
class metatype::basic_instance : public metatype
{
protected:
	inline ~basic_instance()
	{ }
	basic_instance(size_t post);
public:
	int convert(int , void *) __MPT_OVERRIDE;
	
	void unref() __MPT_OVERRIDE;
	basic_instance *clone() const __MPT_OVERRIDE;
	
	bool set(const char *, int);
	
	static basic_instance *create(const char *, int);
};
/* data metatype with typed content */
class metatype::generic_instance : public metatype
{
public:
	static generic_instance *create(int, const void *);
	
	int convert(int , void *) __MPT_OVERRIDE;
	
	uintptr_t addref() __MPT_OVERRIDE;
	void unref() __MPT_OVERRIDE;
	generic_instance *clone() const __MPT_OVERRIDE;
private:
	refcount _ref;
protected:
	generic_instance(const type_traits *, int );
	static generic_instance *create(int, const type_traits *, const void *);
	virtual ~generic_instance();
	void *_val;
	const type_traits *_traits;
	unsigned int _valtype;
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
		int me = value_id();
		if (!type) {
			if (dest) {
				*static_cast<const uint8_t **>(dest) = 0;
			}
			return me;
		}
		if (type == TypeMetaPtr) {
			*static_cast<metatype **>(dest) = this;
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
	metatype *clone() const __MPT_OVERRIDE
	{
		return new meta_value(_val);
	}
	static int value_id() {
		static int _valtype = 0;
		if (_valtype > 0) {
			return _valtype;
		}
		int type = type_properties<T>::id(true);
		if (type < 0) {
			type = TypeMetaPtr;
		}
		return _valtype = type;
	}
protected:
	T _val;
};
template<typename T>
class type_properties<meta_value<T> *>
{
protected:
	type_properties();
public:
	static inline __MPT_CONST_EXPR int id() {
		return TypeMetaPtr;
	}
	static inline const struct type_traits *traits(void) {
		return type_traits::get(id());
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
		return type_properties<T>::id();
	}
protected:
	span<const T> _d;
	int _pos;
};
template <typename T>
class type_properties<source<T> *>
{
protected:
	type_properties();
public:
	static inline __MPT_CONST_EXPR int id(bool) {
		return TypeIteratorPtr;
	}
	static inline const struct type_traits *traits(void) {
		return type_traits::get(id(true));
	}
};
#endif

__MPT_EXTDECL_BEGIN

/* meta reference type information */
extern const MPT_STRUCT(type_traits) *mpt_meta_reference_traits(void);

/* create meta type element */
extern MPT_INTERFACE(metatype) *mpt_meta_new(const MPT_STRUCT(value) *);
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
extern MPT_INTERFACE(metatype) *mpt_iterator_string(const char *, const char *__MPT_DEFPAR(0));

/* get value from iterator and advance */
extern int mpt_iterator_consume(MPT_INTERFACE(iterator) *, int , void *);


__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_META_H */
