/*!
 * MPT core library
 *  object extensions
 */

#ifndef _MPT_OBJECT_H
#define _MPT_OBJECT_H  @INTERFACE_VERSION@

#include "core.h"

__MPT_NAMESPACE_BEGIN

MPT_INTERFACE(metatype);

/*! generic object interface */
#ifdef __cplusplus
MPT_INTERFACE(object) : public unrefable
{
protected:
	inline ~object() {}
public:
	enum { Type = TypeObject };
	
	class iterator;
	class const_iterator;
	class Property;
	
	class iterator begin();
	class iterator end();
	
	class const_iterator const_begin() const;
	class const_iterator const_end() const;
	class const_iterator begin() const;
	class const_iterator end() const;
	
	bool set(const char *, const value &, logger * = logger::defaultInstance());
	bool setProperties(const object &, logger * = logger::defaultInstance());
	
	inline int type() const
	{ return property(0); }
	
	virtual uintptr_t addref();
	virtual int property(struct property *) const = 0;
	virtual int setProperty(const char *, const metatype * = 0) = 0;
};
#else
MPT_INTERFACE(object);
MPT_INTERFACE_VPTR(object) {
	MPT_INTERFACE_VPTR(unrefable) ref;
	uintptr_t (*addref)(MPT_INTERFACE(object) *);
	int (*property)(const MPT_INTERFACE(object) *, MPT_STRUCT(property) *);
	int (*setProperty)(MPT_INTERFACE(object) *, const char *, const MPT_INTERFACE(metatype) *);
};MPT_INTERFACE(object) {
	const MPT_INTERFACE_VPTR(object) *_vptr;
};
#endif

__MPT_EXTDECL_BEGIN

/* get object type name */
extern const char *mpt_object_typename(MPT_INTERFACE(object) *);

/* loop trough metatype/generic properties */
extern int mpt_object_foreach(const MPT_INTERFACE(object) *, MPT_TYPE(PropertyHandler) , void *, int __MPT_DEFPAR(-1));

/* set metatype property to match argument */
extern int mpt_object_set_iterator(MPT_INTERFACE(object) *, const char *, MPT_INTERFACE(iterator) *);
extern int mpt_object_set_string(MPT_INTERFACE(object) *, const char *, const char *, const char * __MPT_DEFPAR(0));
extern int mpt_object_set_value(MPT_INTERFACE(object) *, const char *, MPT_STRUCT(value) *);
extern int mpt_object_vset(MPT_INTERFACE(object) *, const char *, const char *, va_list);
extern int mpt_object_set (MPT_INTERFACE(object) *, const char *, const char *, ... );


__MPT_EXTDECL_END

#ifdef __cplusplus
template<> int Item<object>::type();

inline uintptr_t object::addref()
{ return 0; }

class object::const_iterator
{
public:
    inline const_iterator(const class object &obj) : _ref(obj), _pos(-1)
    { }

    inline const_iterator &operator ++()
    {
        if (_pos >= 0 && !select(++_pos)) {
            setInvalid();
        }
        return *this;
    }
    inline bool operator ==(const const_iterator &cmp) const
    {
         return _pos == cmp._pos;
    }
    inline void setInvalid()
    {
        _prop.name = 0;
        _pos = -1;
    }
    inline const char *name()
    {
        return _prop.name;
    }
    inline const struct property &operator *() const
    {
        return _prop;
    }
    bool select(uintptr_t);
protected:
    const class object &_ref;
    intptr_t _pos;
    struct ::mpt::property _prop;
};

class object::iterator
{
public:
    inline iterator(class object &obj) : _ref(obj), _pos(-1), _name(0)
    { }

    inline iterator &operator ++()
    {
        if (_pos >= 0 && !select(++_pos)) {
            setInvalid();
        }
        return *this;
    }
    inline bool operator ==(const iterator &cmp) const
    {
         return _pos == cmp._pos;
    }
    inline void setInvalid()
    {
        _name = 0;
        _pos = -1;
    }
    inline const char *name()
    {
        return _name;
    }
    inline const char *operator *() const
    {
        return _name;
    }
    bool select(uintptr_t);
protected:
    const class object &_ref;
    intptr_t _pos;
    const char *_name;
};

inline object::iterator object::begin()
{
    iterator it(*this);
    it.select(0);
    return it;
}
inline object::iterator object::end()
{
    return iterator(*this);
}

inline object::const_iterator object::const_begin() const
{
    const_iterator it(*this);
    it.select(0);
    return it;
}
inline object::const_iterator object::const_end() const
{
    return const_iterator(*this);
}
inline object::const_iterator object::begin() const
{
    return const_begin();
}
inline object::const_iterator object::end() const
{
    return const_end();
}

class object::Property : Reference<object>
{
public:
    Property(const Reference<object> &);
    Property(object * = 0);
    ~Property();

    inline operator const struct property&() const
    { return _prop; }

    inline operator const struct value&() const
    { return _prop.val; }

    bool select(const char * = 0);
    bool select(int);

    bool set(const metatype &);
    bool set(const struct value &);

    Property & operator= (const char *val);
    Property & operator= (const metatype &meta);
    Property & operator= (const struct property &);

    inline Property & operator= (const value &v)
    { if (!set(v)) _prop.name = 0; return *this; }
    
    
    template <typename T>
    Property & operator= (const T &v)
    {
        static const char _fmt[2] = { static_cast<char>(typeIdentifier<T>()) };
        value val;
        val.set(_fmt, &v);
        if (!set(val)) _prop.name = 0;
        return *this;
    }

protected:
    struct property _prop;
    friend class Object;

private:
    Property & operator= (const Property &);
};

struct node;
class Object : protected Item<object>
{
public:
    /* create storage */
    Object(Object &);
    Object(object * = 0);
    Object(const Reference<object> &);
    virtual ~Object();

    /* get/replace meta pointer */
    Object & operator=(Object &);
    Object & operator=(Reference<object> const &);

    /* convert from other object type */
    template <typename T>
    Object & operator=(Reference<T> const from)
    {
        Reference<T> ref = from;
        T *ptr = ref.pointer();
        if (ptr && setPointer(ptr)) ref.detach();
        return *this;
    }

    /* get/replace meta pointer */
    inline object *pointer() const
    { return _ref; }
    virtual const Reference<object> &ref();
    virtual bool setPointer(object *);

    /* object store identifier */
    inline const char *name() const
    { return Item<object>::name(); }
    virtual bool setName(const char *, int = -1);

    /* get property by name/position */
    object::Property operator [](const char *);
    object::Property operator [](int);
    int type();

    /* object name hash */
    inline uintptr_t hash() const
    { return _hash; }
    /* name and object data printable */
    inline bool printable() const
    { return name() && (!_ref || _ref->type() == 's'); }

    /* get properties from node list */
    const node *getProperties(const node *, PropertyHandler , void *) const;
    /* set non-default/all child entrys */
    int setProperties(node *) const;
    int setAllProperties(node *) const;

    /* get next node with default/unknown property */
    node *getDefault(const node *) const;
    node *getAlien(const node *) const;

protected:
    uintptr_t _hash;
};

/*! interface to generic groups of metatypes elements */
class Group : public object
{
public:
    enum { Type = TypeGroup };
    
    int property(struct property *) const __MPT_OVERRIDE;
    int setProperty(const char *, const metatype *) __MPT_OVERRIDE;
    
    virtual const Item<object> *item(size_t pos) const;
    virtual Item<object> *append(object *);
    virtual size_t clear(const unrefable * = 0);
    virtual bool bind(const Relation &from, logger * = logger::defaultInstance());
    
    virtual void *toType(int);
    virtual const Transform &transform();
    
    bool addItems(node *head, const Relation *from = 0, logger * = logger::defaultInstance());
    
protected:
    inline ~Group() {}
    virtual object *create(const char *, int = -1);
};
template<> inline __MPT_CONST_EXPR int typeIdentifier<Group>() { return Group::Type; }

/*! Relation implemetation using Group as current element */
class GroupRelation : public Relation
{
public:
    inline GroupRelation(const Group &g, const Relation *p = 0, char sep = '.') : Relation(p), _curr(g), _sep(sep)
    { }
    virtual ~GroupRelation()
    { }
    object *find(int type, const char *, int = -1) const __MPT_OVERRIDE;
protected:
    const Group &_curr;
    char _sep;
};
#endif /* C++ */

__MPT_NAMESPACE_END

#ifdef __cplusplus
inline bool operator !=(const mpt::object::iterator &i1, const mpt::object::iterator &i2)
{
    return !(i1 == i2);
}
inline bool operator !=(const mpt::object::const_iterator &i1, const mpt::object::const_iterator &i2)
{
    return !(i1 == i2);
}

#endif /* C++ */

#endif /* MPT_OBJECT_H */
