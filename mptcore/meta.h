/*!
 * MPT core library
 *  basic types and definitions
 */

#ifndef _MPT_META_H
#define _MPT_META_H  @INTERFACE_VERSION@

#include "core.h"

__MPT_NAMESPACE_BEGIN

/*! generic metatype interface */
MPT_INTERFACE(metatype)
#ifdef __cplusplus
{
protected:
	inline ~metatype() {}
public:
	enum { Type = TypeMeta };
	
	const char *string();
	
	template <typename T>
	inline T *cast()
	{
	    int t = typeIdentifier<T>();
	    if (!mpt_valsize(t)) return 0;
	    T *ptr;
	    if (conv(t, &ptr) < 0) return 0;
	    return ptr;
	}
	inline operator const char *()
	{ return string(); }
	
	static metatype *create(size_t size);
	
	virtual void unref() = 0;
	virtual int assign(const value *);
	virtual int conv(int, void *);
	virtual metatype *clone() const;
	
	inline int type()
	{ return conv(0, 0); }
#else
; MPT_INTERFACE_VPTR(metatype)
{
	void (*unref)(MPT_INTERFACE(metatype) *);
	int (*assign)(MPT_INTERFACE(metatype) *, const MPT_INTERFACE(value) *);
	int (*conv)(MPT_INTERFACE(metatype) *, int, void *);
	MPT_INTERFACE(metatype) *(*clone)(const MPT_INTERFACE(metatype) *);
}; MPT_INTERFACE(metatype) {
	const MPT_INTERFACE_VPTR(metatype) *_vptr;
#endif
};


#ifdef __cplusplus
inline metatype *metatype::clone() const
{ return 0; }

/* specialize metatype string cast */
template <> inline const char *metatype::cast<const char>()
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
class Metatype : public metatype
{
public:
    void unref();
    int assign(const value *);
    int conv(int, void *);
    metatype *clone();

    Slice<const char> data() const;
    class Small;
    class Big;

    static Metatype *create(size_t size);

protected:
    Metatype(size_t post = 0);
    virtual ~Metatype();

    uint64_t _info;
};

class Metatype::Small : public Metatype
{
public:
    Small() : Metatype(sizeof(data))
    { }
protected:
    friend class Metatype;
    int8_t data[64-sizeof(Metatype)];
};
class Metatype::Big : public Metatype
{
public:
    Big() : Metatype(sizeof(data))
    { }
protected:
    friend class Metatype;
    int8_t data[256-sizeof(Metatype)];
};

template <typename T>
class Source : public metatype
{
public:
    Source(const T *val, size_t len = 1) : _d(val, len)
    { }
    virtual ~Source()
    { }

    void unref()
    { delete this; }

    int conv(int type, void *dest)
    {
        if (!_d.len()) return -2;
        const T *val = _d.base();
        type = convert((const void **) &val, typeIdentifier<T>(), dest, type);
        if ((type < 0) || !dest) return type;
        _d.shift(1);
        return sizeof(T);
    }
protected:
    Slice<const T> _d;
};
#endif

__MPT_EXTDECL_BEGIN
/* create meta type element */
extern MPT_INTERFACE(metatype) *mpt_meta_new(size_t);

/* get node/metatype text/raw data */
extern const char *mpt_meta_data(MPT_INTERFACE(metatype) *, size_t *__MPT_DEFPAR(0));
/* initialize geninfo data */
extern int _mpt_geninfo_init(void *, size_t);
/* operations on geninfo data */
extern int _mpt_geninfo_value(uint64_t *, const MPT_STRUCT(value) *);
extern int _mpt_geninfo_line(const uint64_t *);
extern int _mpt_geninfo_conv(const uint64_t *, int , void *);
/* create new metatype with data */
extern MPT_INTERFACE(metatype) *_mpt_geninfo_clone(const uint64_t *);

__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_META_H */
