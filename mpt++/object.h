/*!
 * MPT C++ library
 *  object extensions
 */

#ifndef _MPT_OBJECT_H
#define _MPT_OBJECT_H  @INTERFACE_VERSION@

#include <ostream>

#include "core.h"

namespace mpt {

struct color;
struct lineattr;

class property::iterator : property
{
public:
    iterator();

    inline const char *name() const
    {
        return property::name;
    }
    inline const char *desc() const
    {
        return property::desc;
    }
    inline operator const value&() const
    {
        return property::val;
    }
    inline operator const property *() const
    {
        return this;
    }
    inline bool operator ==(const iterator &cmp) const
    {
         return _pos == cmp._pos;
    }
protected:
    friend class object::iterator;
    friend class object::const_iterator;
    bool select(const object &, uintptr_t);
    intptr_t _pos;
};

class object::iterator : public property::iterator
{
public:
    iterator(class object *);

    inline iterator &operator ++()
    {
        if (_ref && _pos >= 0) {
            select(*_ref, _pos+1);
        }
        return *this;
    }
    inline class object *object() const
    {
        return _ref;
    }
    inline bool operator ==(const iterator &cmp) const
    {
         return _pos == cmp._pos;
    }
protected:
    class object *_ref;
};

class object::const_iterator : public property::iterator
{
public:
    const_iterator(const class object *);

    inline iterator &operator ++()
    {
        if (_ref && _pos >= 0) {
            select(*_ref, _pos+1);
        }
        return *this;
    }
    inline const class object *object() const
    {
        return _ref;
    }
    inline bool operator ==(const const_iterator &cmp) const
    {
         return _pos == cmp._pos;
    }
protected:
    const class object *_ref;
};

inline object::iterator object::begin()
{
    return iterator(this);
}
inline object::iterator object::end()
{
    return iterator(0);
}

inline object::const_iterator object::const_begin() const
{
    return const_iterator(this);
}
inline object::const_iterator object::const_end() const
{
    return const_iterator(0);
}
inline object::const_iterator object::begin() const
{
    return const_iterator(this);
}
inline object::const_iterator object::end() const
{
    return const_iterator(0);
}

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
        if (!Reference<T>::_ref) Reference<T>::_ref = new typename Reference<T>::instance;
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

inline bool operator !=(const mpt::object::iterator &i1, const mpt::object::iterator &i2)
{
    return !(i1 == i2);
}
inline bool operator !=(const mpt::object::const_iterator &i1, const mpt::object::const_iterator &i2)
{
    return !(i1 == i2);
}

std::basic_ostream<char> &operator<<(std::basic_ostream<char> &, const mpt::value &);

#endif // MPT_OBJECT_H
