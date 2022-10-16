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
	class basic;
	class generic;
	
	template <typename T>
	class value;
	
	static metatype *create(const ::mpt::value &);
	static metatype *create(const char *, int = -1);
	
	int convert(int , void *) __MPT_OVERRIDE;
	
	virtual void unref() = 0;
	virtual uintptr_t addref();
	virtual metatype *clone() const = 0;
	
	template <typename T>
	bool get(T &val)
	{
		int type = type_properties<T>::id(true);
		return (type > 0) && (convert(type, &val) >= 0);
	}
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

inline uintptr_t metatype::addref() {
	return 0;
}
inline metatype *metatype::clone() const {
	return 0;
}
#else
MPT_STRUCT(value);
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

#ifdef __cplusplus
/* basic metatype to support typeinfo */
class metatype::basic : public metatype
{
protected:
	inline ~basic()
	{ }
	basic(size_t post);
public:
	int convert(int , void *) __MPT_OVERRIDE;
	
	void unref() __MPT_OVERRIDE;
	basic *clone() const __MPT_OVERRIDE;
	
	bool set(const char *, int);
	
	static basic *create(const char *, int = -1);
};
/* data metatype with typed content */
class metatype::generic : public metatype
{
public:
	static generic *create(int, const void *);
	
	int convert(int , void *) __MPT_OVERRIDE;
	
	uintptr_t addref() __MPT_OVERRIDE;
	void unref() __MPT_OVERRIDE;
	generic *clone() const __MPT_OVERRIDE;
private:
	refcount _ref;
protected:
	generic(const type_traits *, int );
	static generic *create(int, const type_traits *, const void *);
	virtual ~generic();
	void *_val;
	const type_traits *_traits;
	unsigned int _valtype;
};
/* generic implementation for metatype */
template <typename T>
class metatype::value : public metatype
{
public:
	inline value(const T *val = 0)
	{
		if (val) _val = (*val);
	}
	inline value(const T &val) : _val(val)
	{ }
	virtual ~value()
	{ }
	void unref() __MPT_OVERRIDE
	{
		delete this;
	}
	int convert(int type, void *dest) __MPT_OVERRIDE
	{
		int me = type_properties<T>::id(true);
		if (me < 0) {
			me = type_properties<metatype *>::id(true);
		}
		if (!type) {
			if (dest) {
				*static_cast<const uint8_t **>(dest) = 0;
			}
			return me;
		}
		if (type == type_properties<metatype *>::id(true)) {
			*static_cast<metatype **>(dest) = this;
			return me;
		}
		if (me < 0 || type != me) {
			return BadType;
		}
		if (dest) {
			*static_cast<T *>(dest) = _val;
		}
		return me;
	}
	value *clone() const __MPT_OVERRIDE
	{
		return new value(_val);
	}
protected:
	T _val;
};
template<typename T>
class type_properties<metatype::value<T> *>
{
protected:
	type_properties();
public:
	static inline __MPT_CONST_EXPR int id(bool) {
		return TypeMetaPtr;
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
/* set metatype reference */
#ifdef __cplusplus
extern int mpt_meta_set(reference<metatype> &, const MPT_STRUCT(value) *);
#else
extern int mpt_meta_set(MPT_INTERFACE(metatype) **, const MPT_STRUCT(value) *);
#endif

/* creat basic text small metatype */
extern MPT_INTERFACE(metatype) *mpt_meta_geninfo(size_t);

/* create meta type element */
extern MPT_INTERFACE(metatype) *mpt_metatype_default();

/* assign to value via iterator */
extern MPT_INTERFACE(metatype) *mpt_iterator_string(const char *, const char *__MPT_DEFPAR(0));

/* initialize geninfo data */
extern int _mpt_geninfo_size(size_t);
extern int _mpt_geninfo_init(void *, size_t);
/* operations on geninfo data */
extern int _mpt_geninfo_set(void *, const char *, int __MPT_DEFPAR(-1));
extern int _mpt_geninfo_flags(const void *, int);
extern int _mpt_geninfo_conv(const void *, int , void *);
/* clone geninfo content */
extern MPT_INTERFACE(metatype) *_mpt_geninfo_clone(const void *);


__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_META_H */
