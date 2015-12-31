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

class Property : Reference<object>
{
public:
    Property(const Reference<object> &);
    Property(object * = 0);
    ~Property();

    inline operator const property&() const
    { return _prop; }

    inline operator const value&() const
    { return _prop.val; }

    bool select(const char * = 0);
    bool select(int);

    bool set(metatype &);
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
    friend class Object;

private:
    Property & operator= (const Property &);
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

class Object : protected Item<object>
{
public:
    // create storage
    Object(Object &);
    Object(object * = 0);
    Object(const Reference<object> &);
    virtual ~Object();

    // get/replace meta pointer
    Object & operator=(Object &);
    Object & operator=(Reference<object> const &);

    // convert from other object type
    template <typename T>
    Object & operator=(Reference<T> const from)
    {
        Reference<T> ref = from;
        if (ref && setObject(ref)) ref.detach();
        return *this;
    }

    // get/replace meta pointer
    inline operator object*() const
    { return _ref; }
    virtual const Reference<object> &ref();
    virtual bool setObject(object *);

    // object store identifier
    inline const char *name() const
    { return Item::name(); }
    virtual bool setName(const char *, int = -1);

    // get property by name/position
    Property operator [](const char *);
    Property operator [](int);
    int type();

    // object name hash
    inline long hash() const
    { return _hash; }
    // name and object data printable
    inline bool printable() const
    { return name() && (!_ref || _ref->type() == 's'); }

    // get properties from node list
    const node *getProperties(const node *, PropertyHandler , void *) const;
    // set non-default/all child entrys
    int setProperties(node *) const;
    int setAllProperties(node *) const;

    // get next node with default/unknown property
    node *getDefault(const node *) const;
    node *getAlien(const node *) const;

protected:
    long _hash;
};

} /* namespace mpt */

#endif // MPT_OBJECT_H
