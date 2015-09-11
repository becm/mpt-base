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

template <typename T>
class Value : public value
{
public:
    inline Value(const T &v) : _val(v)
    {
        static const char _fmt[2] = { static_cast<char>(typeIdentifier<T>()) };
        fmt = _fmt;
        ptr = &_val;
    }
private:
    T _val;
};

class Property : Reference<metatype>
{
public:
    Property(const Reference<metatype> &);
    inline ~Property()
    { }

    inline operator const property&() const
    { return _prop; }

    bool select(const char * = 0);
    bool select(int);

    bool set(source &);
    bool set(const struct value &);

    Property & operator= (const char *val);
    Property & operator= (metatype &meta);

    inline Property & operator= (const struct value &v)
    { if (!set(v)) _prop.name = 0; return *this; }
    
    Property & operator= (const property &);
    Property & operator= (const Property &);

    template <typename T>
    inline Property & operator= (const T &v)
    { if (!set(Value<T>(v))) _prop.name = 0; return *this; }

protected:
    property _prop;
    friend class Object;
};

template <typename T>
class Source : public source, Slice<const T>
{
public:
    Source(const T *val, size_t len = 1) : Slice<const T>(val, len)
    { }

    virtual ~Source()
    { }

    int unref()
    { return 0; }

    int conv(int type, void *dest)
    {
        if (!Slice<const T>::_len) return -2;
        const T *val = Slice<const T>::_base;
        if ((type = convert((const void **) &val, typeIdentifier<T>(), dest, type)) < 0) return -1;
        if (!dest) return type;
        Slice<const T>::_len -= sizeof(T);
        Slice<const T>::_base = val;
        return sizeof(T);
    }
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
    inline int type() const
    { return _ref ? _ref->type() : -1; }

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
    int property(struct property *, source * = 0);
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
