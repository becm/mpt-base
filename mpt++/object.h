/*!
 * MPT C++ library
 *  metatype extensions
 */

#ifndef _MPT_OBJECT_H
#define _MPT_OBJECT_H  201502

#include "core.h"

namespace mpt {

struct color;
struct lineattr;

class Property
{
public:
    Property(const Reference<metatype> &);
    Property(object * = 0);
    ~Property();

    inline operator const property&() const
    { return _prop; }

    inline operator const value&() const
    { return _prop.val; }

    bool select(const char * = 0);
    bool select(int);

    bool set(source &);
    bool set(const struct value &);

    Property & operator= (const char *val);
    Property & operator= (metatype &meta);
    Property & operator= (const property &);

    inline Property & operator= (const value &v)
    { if (!set(v)) _prop.name = 0; return *this; }
    
    
    template <typename T>
    Property & operator= (const T &v)
    {
        static const char _fmt[2] = { static_cast<char>(typeIdentifier<T>()) };
        if (!set(value(_fmt, &v))) _prop.name = 0;
        return *this;
    }

protected:
    property _prop;
    object *_obj;
    friend class Object;

private:
    Property & operator= (const Property &);
};

template <typename T>
class Source : public source
{
public:
    Source(const T *val, size_t len = 1) : _d(val, len)
    { }
    virtual ~Source()
    { }

    int unref()
    { delete this; return 0; }

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

// auto-create reference
template <typename T>
class Container : protected Reference<T>
{
public:
    inline Container(T *ref = 0) : Reference<T>(ref)
    { }
    inline Container(const Reference<T> &ref) : Reference<T>(ref)
    { }
    virtual ~Container()
    { }
    virtual const Reference<T> &ref()
    {
        if (!Reference<T>::_ref) Reference<T>::_ref = new T;
        return *this;
    }
    inline operator T*() const
    { return Reference<T>::_ref; }
};

class Object : protected Item<metatype>
{
public:
    // create storage
    Object(Object &);
    Object(const Reference<metatype> & = Reference<metatype>());
    Object(size_t);
    virtual ~Object();

    // get/replace meta pointer
    Object & operator=(Object &);
    Object & operator=(Reference<metatype> const &);

    // convert from other metatype
    template <typename T>
    Object & operator=(Reference<T> const from)
    {
        Reference<T> ref = from;
        if (ref && setMeta(ref)) ref.detach();
        return *this;
    }

    // get/replace meta pointer
    inline operator metatype*() const
    { return _ref; }
    virtual const Reference<metatype> &ref();
    virtual bool setMeta(metatype *);

    // object store identifier
    inline const char *name() const
    { return Item::name(); }
    virtual bool setName(const char *, int = -1);

    // get property by name/position
    Property operator [](const char *);
    Property operator [](int);
    int type();

    // metatype name hash
    inline long hash() const
    { return _hash; }
    // name and metatype data printable
    inline bool printable() const
    { return name() && (!_ref || _ref->typecast('s')); }

    // get properties from node list
    const node *getProperties(const node *, PropertyHandler , void *) const;
    // set non-default/all child entrys
    int setProperties(node *) const;
    int setAllProperties(node *) const;

    // get next node with default/unknown property
    node *getDefault(const node *) const;
    node *getAlien(const node *) const;

    // get default metatype
    static const Reference<metatype> &defaultReference(void);

protected:
    long _hash;
};


// generic implementation for metatype
class MetatypeGeneric : public metatype
{
public:
    int unref();
    metatype *addref();
    int assign(const value *);
    void *typecast(int);

    Slice<const char> data(void) const;
    class Small;
    class Big;

    static MetatypeGeneric *create(size_t size);

protected:
    MetatypeGeneric(size_t post = 0, uintptr_t ref = 1);
    virtual ~MetatypeGeneric();

    uint64_t _info;
};

class MetatypeGeneric::Small : public MetatypeGeneric
{
public:
    Small(uintptr_t ref = 1) : MetatypeGeneric(sizeof(data), ref)
    { }
protected:
    friend class MetatypeGeneric;
    int8_t data[64-sizeof(MetatypeGeneric)];
};
class MetatypeGeneric::Big : public MetatypeGeneric
{
public:
    Big(uintptr_t ref = 1) : MetatypeGeneric(sizeof(data), ref)
    { }
protected:
    friend class MetatypeGeneric;
    int8_t data[256-sizeof(MetatypeGeneric)];
};

} /* namespace mpt */

#endif // MPT_OBJECT_H
