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
	
	template <typename T>
	static metatype *create(const T &);
	
	int convert(value_t , void *) __MPT_OVERRIDE;
	
	virtual void unref() = 0;
	virtual uintptr_t addref();
	virtual metatype *clone() const = 0;
	
	static const struct named_traits *pointer_traits(void);
	static const struct type_traits *reference_traits(void);
};
template <> inline __MPT_CONST_TYPE int type_properties<metatype *>::id(bool) {
	return TypeMetaPtr;
}
template <> inline const struct type_traits *type_properties<metatype *>::traits() {
	static const struct named_traits *nt = metatype::pointer_traits();
	return &nt->traits;
}

template <> inline __MPT_CONST_TYPE int type_properties<reference<metatype> >::id(bool) {
	return TypeMetaRef;
}
template <> inline const struct type_traits *type_properties<reference<metatype> >::traits() {
	static const struct type_traits *t = metatype::reference_traits();
	return t;
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
/* basic metatype with string data */
class metatype::basic : public metatype
{
protected:
	inline ~basic()
	{ }
	basic(size_t post);
public:
	int convert(value_t , void *) __MPT_OVERRIDE;
	
	void unref() __MPT_OVERRIDE;
	basic *clone() const __MPT_OVERRIDE;
	
	bool set(const char *, int);
	
	static basic *create(const char *, int = -1);
	static const named_traits *pointer_traits(bool = true);
};
template <>
class type_properties<metatype::basic *>
{
protected:
	type_properties();
public:
	static inline int id(bool obtain = true) {
		const named_traits *t = metatype::basic::pointer_traits(obtain);
		if (t) {
			return t->type;
		}
		return obtain ? BadOperation : BadType;
	}
	static inline const struct type_traits *traits(void) {
		const named_traits *t = metatype::basic::pointer_traits(true);
		return t ? &t->traits : 0;
	}
};

/* data metatype with typed content */
class metatype::generic : public metatype
{
public:
	int convert(value_t , void *) __MPT_OVERRIDE;
	
	uintptr_t addref() __MPT_OVERRIDE;
	void unref() __MPT_OVERRIDE;
	generic *clone() const __MPT_OVERRIDE;
	
	static generic *create(value_t, const void *);
	static const named_traits *pointer_traits(bool = true);
private:
	refcount _ref;
protected:
	static generic *create(value_t , const void *, const type_traits &);
	
	generic();
	virtual ~generic();
	const type_traits *_traits;
	void *_val;
	value_t _type;
};
template <>
class type_properties<metatype::generic *>
{
protected:
	type_properties();
public:
	static inline int id(bool obtain = true) {
		const named_traits *t = metatype::generic::pointer_traits(obtain);
		if (t) {
			return t->type;
		}
		return obtain ? BadOperation : BadType;
		
	}
	static inline const struct type_traits *traits(void) {
		const named_traits *t = metatype::generic::pointer_traits(true);
		return t ? &t->traits : 0;
	}
};

/* metatype for generic content */
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
	int convert(value_t type, void *dest) __MPT_OVERRIDE
	{
		int type_val = type_properties<T>::id(true);
		if (!type) {
			int ret = metatype::convert(0, dest);
			return type_val > 0 ? type_val : ret;
		}
		int type_meta = type_properties<value<T> *>::id(false);
		if (type == TypeMetaPtr) {
			if (dest) {
				*static_cast<metatype **>(dest) = this;
			}
			return type_meta > 0 ? type_meta : static_cast<int>(TypeMetaPtr);
		}
		if (type_meta > 0 && type == static_cast<value_t>(type_meta)) {
			if (dest) {
				*static_cast<metatype **>(dest) = this;
			}
			return type_val > 0 ? type_val : static_cast<int>(TypeMetaPtr);
		}
		if (type_val > 0) {
			if (type == static_cast<value_t>(type_val)) {
				if (dest) {
					*static_cast<T *>(dest) = _val;
				}
				return type_meta > 0 ? type_meta : static_cast<int>(TypeMetaPtr);
			}
			::mpt::value val;
			if (!val.set(type_val, &_val)) {
				return BadType;
			}
			return val.convert(type, dest);
		}
		return BadType;
	}
	value *clone() const __MPT_OVERRIDE
	{
		return new value(_val);
	}
	
	static const named_traits *pointer_traits(bool obtain = true)
	{
		static const struct named_traits *traits = 0;
		if (!traits && obtain) {
			traits = type_traits::add_metatype();
		}
		return traits;
	}
protected:
	T _val;
};
template <typename T>
class type_properties<metatype::value<T> *>
{
protected:
	type_properties();
public:
	static inline int id(bool obtain = false) {
		const named_traits *t = metatype::value<T>::pointer_traits(obtain);
		if (t) {
			return t->type;
		}
		return obtain ? BadOperation : BadType;
	}
	static inline const struct type_traits *traits(void) {
		const named_traits *t = metatype::value<T>::pointer_traits(true);
		return t ? &t->traits : 0;
	}
};

template <typename T>
metatype *metatype::create(const T&val)
{
	return new metatype::value<T>(val);
}
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
extern int _mpt_geninfo_conv(const void *, MPT_TYPE(value) , void *);
/* clone geninfo content */
extern MPT_INTERFACE(metatype) *_mpt_geninfo_clone(const void *);


__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_META_H */
