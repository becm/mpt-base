/*!
 * MPT core library
 *  basic types and definitions
 */

#ifndef _MPT_META_H
#define _MPT_META_H  @INTERFACE_VERSION@

#include "core.h"

__MPT_NAMESPACE_BEGIN

/*! generic metatype interface */
#ifdef __cplusplus
MPT_INTERFACE(metatype) : public unrefable
{
protected:
	inline ~metatype() {}
public:
	enum { Type = TypeMeta };
	
	class Basic;
	
	const char *string() const;
	
	template <typename T>
	inline T *cast() const
	{
	    static const int t = typeIdentifier<T>();
	    if (mpt_valsize(t)) return 0;
	    T *ptr;
	    if (conv(t, &ptr) < 0) return 0;
	    return ptr;
	}
	inline operator const char *() const
	{ return string(); }
	
	static metatype *create(value);
	static metatype *create(int, const void *);
	
	virtual int conv(int , void *) const = 0;
	virtual metatype *clone() const;
	
	inline int type() const
	{ return conv(0, 0); }
};
#else
MPT_INTERFACE(metatype);
MPT_INTERFACE_VPTR(metatype)
{
	MPT_INTERFACE_VPTR(unrefable) ref;
	int (*conv)(const MPT_INTERFACE(metatype) *, int , void *);
	MPT_INTERFACE(metatype) *(*clone)(const MPT_INTERFACE(metatype) *);
}; MPT_INTERFACE(metatype) {
	const MPT_INTERFACE_VPTR(metatype) *_vptr;
};
#endif

/*! generic iterator interface */
#ifdef __cplusplus
MPT_INTERFACE(iterator) : public unrefable
{
protected:
	inline ~iterator() {}
public:
	virtual int get(int, void *) = 0;
	virtual int advance();
	virtual int reset();
};
#else
MPT_INTERFACE(iterator);
MPT_INTERFACE_VPTR(iterator)
{
	MPT_INTERFACE_VPTR(unrefable) ref;
	int (*get)(MPT_INTERFACE(iterator) *, int , void *);
	int (*advance)(MPT_INTERFACE(iterator) *);
	int (*reset)(MPT_INTERFACE(iterator) *);
}; MPT_INTERFACE(iterator) {
	const MPT_INTERFACE_VPTR(iterator) *_vptr;
};
#endif

#ifdef __cplusplus
inline metatype *metatype::clone() const
{ return 0; }
inline int iterator::advance()
{ return 0; }
inline int iterator::reset()
{ return 0; }

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

/* specialize metatype string cast */
template <> inline const char *metatype::cast<const char>() const
{ return string(); }

/* special copy for metatype */
template <> inline Reference<metatype> & Reference<metatype>::operator= (Reference<metatype> const &ref)
{
    metatype *r = ref._ref;
    if (r) r = r->clone();
    if (_ref) _ref->unref();
    _ref = r;
    return *this;
}
/* generic implementation for metatype */
template <typename T>
class Metatype : public metatype
{
public:
    inline Metatype(const T *val = 0) : _val(val ? *val : 0)
    { }
    inline Metatype(const T &val) : _val(val)
    { }
    void unref()
    {
        delete this;
    }
    int conv(int type, void *dest) const
    {
        static const int fmt = typeIdentifier<T>();
        if (!type) {
            if (dest) *static_cast<const char **>(dest) = 0;
            return fmt;
        }
        if (type != fmt) return BadType;
        if (dest) *static_cast<T *>(dest) = _val;
        return fmt;
    }
    metatype *clone() const
    {
         return new Metatype(_val);
    }
protected:
    virtual ~Metatype()
    { }
    T _val;
};

template <typename T>
class Source : public iterator
{
public:
    Source(const T *val, size_t len = 1) : _d(val, len), _pos(0)
    { }
    virtual ~Source()
    { }

    void unref() __MPT_OVERRIDE
    { delete this; }

    int get(int type, void *dest) __MPT_OVERRIDE
    {
        int fmt = this->type();
        const T *val = _d.nth(_pos);
        if (!val) return MissingData;
        type = convert((const void **) &val, fmt, dest, type);
        if (type < 0) return type;
        return fmt;
    }
    int advance() __MPT_OVERRIDE
    {
        int pos = _pos + 1;
        if (pos > _d.size()) return MissingData;
        if (pos == _d.size()) return 0;
        _pos = pos;
        return type();
    }
    int reset() __MPT_OVERRIDE
    {
        _pos = 0;
        return _d.size();
    }
    static int type()
    {
        static int fmt = 0;
        if (!fmt) fmt = typeIdentifier<T>();
        return fmt;
    }
protected:
    Slice<const T> _d;
    int _pos;
};
#endif

__MPT_EXTDECL_BEGIN

/* create meta type element */
extern MPT_INTERFACE(metatype) *mpt_meta_new(MPT_STRUCT(value));

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
/* create new metatype with data */
extern MPT_INTERFACE(metatype) *_mpt_geninfo_clone(const void *);

/* assign to value via iterator */
extern int mpt_iterator_process(MPT_STRUCT(value) *, int (*)(void *, MPT_INTERFACE(iterator) *), void *);
extern MPT_INTERFACE(iterator) *mpt_iterator_value(MPT_STRUCT(value), int __MPT_DEFPAR(-1));
extern MPT_INTERFACE(iterator) *mpt_iterator_string(const char *, const char *__MPT_DEFPAR(0));

/* create metatype for iterator instance */
extern MPT_INTERFACE(metatype) *mpt_iterator_meta(MPT_INTERFACE(iterator) *);

__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_META_H */
