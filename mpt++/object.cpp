/*
 * generic metatype Object wrapper and property extension
 */

#include <stdlib.h>
#include <string.h>

#include "node.h"
#include "object.h"

__MPT_NAMESPACE_BEGIN

template bool setValue(Property &, const int32_t &);
template bool setValue(Property &, const int64_t &);
template bool setValue(Property &, const double  &);

// class extension for basic property
Property::Property(const Reference<metatype> &ref) : Reference<metatype>(ref)
{ }
Property::~Property()
{ }

// assignment operation for metatypes
Property & Property::operator= (metatype &meta)
{
    property pr;
    if (meta.property(&pr) < 0) { this->name = 0; return *this; }
    this->val.fmt = pr.val.fmt;
    this->val.ptr = pr.val.ptr;
    metatype *mt = *this;
    if (!mt || mpt_meta_pset(mt, this) < 0) this->name = 0;
    return *this;
}

// primitive assignment operations
Property & Property::operator= (const int & val)
{ setValue(*this, val); return *this; }
Property & Property::operator= (const long & val)
{ setValue(*this, val); return *this; }
Property & Property::operator= (const double & val)
{ setValue(*this, val); return *this; }

Property & Property::operator= (const char *val)
{
    if (!invalid()) {
        this->val.fmt = 0;
        this->val.ptr = val;
        metatype *mt = *this;
        if (!mt || mpt_meta_pset(mt, this) < 0) this->name = 0;
    }
    return *this;
}

Property & Property::operator= (const property & val)
{
    property tmp = val;
    if (!_ref || mpt_meta_pset(_ref, &tmp) < 0) this->name = 0;
    else { property::operator=(tmp); }
    return *this;
}
Property & Property::operator= (const Property & val)
{
    *this = static_cast<const property &>(val);
    if (_ref == val._ref) return *this;
    if (_ref) _ref->unref();
    _ref = val._ref ? val._ref->addref() : 0;
    property::operator=(val);
    return *this;
}

bool Property::set(const char *name)
{
    if (!_ref) return false;
    property pr(name);
    if (!name && !(pr.name = this->name)) return false;
    if (_ref->property(&pr) < 0) return false;
    property::operator=(pr);
    return true;
}
bool Property::set(int pos)
{
    if (!_ref || pos <= 0) return false;
    property pr(pos);
    if (_ref->property(&pr) < 0) return false;
    property::operator=(pr);
    return true;
}
bool Property::set(source &src)
{
    if (invalid()) return false;
    return (_ref && _ref->property(this, &src) >= 0);
}
bool Property::set(const property &src)
{
    if (!_ref || !src.name) return false;
    property pr = src;
    if (_ref->property(&pr) < 0) return false;
    property::operator=(pr);
    return true;
}

// create storage
Object::Object(Object &other) : Item<metatype>(0), _hash(0)
{
    Reference<metatype> &ref = *this;
    ref = other.ref();
    Slice<const char> d = other.identifier::data();
    if (identifier::setName(d.base(), d.len())) {
        _hash = mpt_hash(d.base(), d.len());
    }
}
Object::Object(const Reference<metatype> &from) : Item<metatype>(0), _hash(0)
{
    Reference<metatype> ref = from;
    _ref = ref.detach();
}
Object::Object(size_t usize) : Item<metatype>(0), _hash(0)
{
    _ref = metatype::create(usize);
}

// clear reference to storage
Object::~Object()
{ }

const Reference<metatype> &Object::ref()
{ return *this; }

// replace existing store
bool Object::setMeta(metatype *meta)
{
    if (!meta || meta == _ref) return false;
    if (_ref) _ref->unref();
    _ref = meta;
    return true;
}
Object & Object::operator =(Reference<metatype> const & from)
{
    Reference<metatype> ref(from);
    if (ref && setMeta(ref)) ref.detach();
    return *this;
}
Object & Object::operator =(Object & other)
{
    Reference<metatype> ref(other.ref());
    if (ref && setMeta(ref)) ref.detach();

    Slice<const char> d = other.identifier::data();
    if (identifier::setName(d.base(), d.len())) {
        _hash = mpt_hash(d.base(), d.len());
    }
    return *this;
}

// set identifier
bool Object::setName(const char *name, int len)
{
    if (!identifier::setName(name, len)) {
        return false;
    }
    Slice<const char> d = identifier::data();

    _hash = mpt_hash(d.base(), d.len());

    return true;
}

// get property by name/position
Property Object::operator [](const char *name)
{
    Property prop(ref());
    if (!prop.set(name)) prop.name = prop.desc = 0;
    return prop;
}
Property Object::operator [](int pos)
{
    Property prop(ref());
    if (!prop.set(pos)) prop.name = prop.desc = 0;
    return prop;
}

struct objectSetContext
{
    metatype *m;
    PropertyHandler check;
    void *cdata;
};

static int metaPropertySet(void *addr, property *pr)
{
    const objectSetContext *con = reinterpret_cast<objectSetContext *>(addr);
    int ret;
    if (!pr->name) {
        return con->check ? con->check(con->cdata, pr) : 1;
    }
    if (!pr->val.ptr) {
        if (!con->check) {
            if (!pr->val.fmt) return 2;
        }
        else {
            int ret = con->check(con->cdata, pr);
            if (ret) return ret;
        }
    }
    if ((ret = mpt_meta_pset(con->m, pr, 0)) < 0) {
        pr->desc = (ret == -1) ? 0 : "";
        return con->check ? con->check(con->cdata, pr) : 3;
    }
    return 0;
}

const node *Object::getProperties(const node *head, PropertyHandler proc, void *pdata) const
{
    objectSetContext con = { _ref, proc, pdata };
    return _ref ? mpt_node_foreach(head, metaPropertySet, &con, TraverseNonLeafs) : 0;
}


static const char globName[] = "metatype\0";
static const char globDesc[] = "default metatype reference\0";

class globalMetatype : public metatype
{
public:
    globalMetatype()  { }
    ~globalMetatype() { }

    metatype *addref() { return this; }
    int unref() { return 0; }
    int property(struct property *p, source *s)
    {
        if (!p) return 0;
        if (p->name || p->desc) return -1;
        p->name = globName;
        p->desc = globDesc;
        p->val.fmt = globName+8;
        p->val.ptr = this;
        return s ? -1 : 0;
    }
    void *typecast(int type)
    {
        return type == metatype::Type ? static_cast<metatype *>(this) : 0;
    }
};

const Reference<metatype> &Object::defaultReference(void)
{
    static const globalMetatype _gMeta;
    static const Reference<metatype> _gMetaRef(const_cast<metatype *>(static_cast<const metatype *>(&_gMeta)));
    return _gMetaRef;
}

__MPT_NAMESPACE_END
