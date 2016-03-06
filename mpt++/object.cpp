/*
 * generic metatype Object wrapper and property extension
 */

#include "node.h"
#include "convert.h"

#include "object.h"

static int stdWrite(void *o, const char *d, size_t n)
{
    ((std::basic_ostream<char> *) o)->write(d, n);
    return n;
}

std::basic_ostream<char> &operator<<(std::basic_ostream<char> &o, const mpt::value &v)
{
    mpt_tostring(&v, stdWrite, &o);
    return o;
}

__MPT_NAMESPACE_BEGIN

property::iterator::iterator() : property((uintptr_t) 0), _pos(0)
{ }

bool property::iterator::select(const object &o, uintptr_t pos)
{
    property::name = 0;
    property::desc = (const char *) pos;
    if (o.property(this) < 0) {
        _pos = -1;
        return false;
    }
    _pos = pos;
    return true;
}

object::const_iterator::const_iterator(const class object *o) : _ref(o)
{
    if (!o || o->property(this) < 0) {
        _pos = -1;
    }
}

object::iterator::iterator(class object *o) : _ref(o)
{
    if (!o || o->property(this) < 0) {
        _pos = -1;
    }
}

// object assignment
bool object::set(const char *name, const value &val, logger *out)
{
    static const char _fname[] = "mpt::object::set";
    int ret;
    if ((ret = mpt_object_pset(this, name, &val)) >= 0) return true;
    if (!out) return false;
    struct property pr;
    if (property(&pr) < 0) pr.name = "object";
    pr.val.fmt = val.fmt;
    if (!(pr.val.ptr = val.ptr)) { pr.val.ptr = ""; pr.val.fmt = 0; }

    if (ret == BadArgument) {
        out->error(_fname, "%s: %s.%s", MPT_tr("bad property"), pr.name, name);
    } else if (ret == BadValue) {
        out->error(_fname, "%s: %s.%s = \"%s\"", MPT_tr("bad property value"), pr.name, name, pr.val.ptr);
    } else if (ret == BadType) {
        out->error(_fname, "%s: %s.%s = <%s>", MPT_tr("bad property type"), pr.name, name, pr.val.fmt);
    }
    return false;
}
struct propertyErrorOut { object *obj; logger *out; };
static int objectSetProperty(void *addr, const property *pr)
{
    struct propertyErrorOut *dat = (propertyErrorOut *) addr;
    if (dat->obj->set(pr->name, pr->val, dat->out)) return 0;
    return dat->out ? 1 : -1;
}
bool object::setProperties(const object &from, logger *out)
{
    propertyErrorOut dat = { this, out };
    return mpt_object_foreach(&from, objectSetProperty, &dat);
}

// class extension for basic property
Property::Property(const Reference<object> &from) : Reference<object>(from)
{ }
Property::Property(object *obj) : Reference<object>(obj)
{ }
Property::~Property()
{ }
// assignment operation for metatypes
Property & Property::operator= (metatype &meta)
{
    if (_ref && _prop.name && !set(meta)) {
        _prop.name = 0;
    }
    return *this;
}
Property & Property::operator= (const char *val)
{
    if (_ref && _prop.name && !set(value((const char *) 0, (const void *) val))) {
        _prop.name = 0;
    }
    return *this;
}

bool Property::select(const char *name)
{
    if (!_ref) return false;
    property pr(name);
    if (_ref->property(&pr) < 0) return false;
    _prop = pr;
    return true;
}
bool Property::select(int pos)
{
    if (!_ref || pos <= 0) return false;
    property pr(pos);
    if (_ref->property(&pr) < 0) return false;
    _prop = pr;
    return true;
}
bool Property::set(metatype &src)
{
    if (!_ref || !_prop.name) return false;
    if (_ref->setProperty(_prop.name, &src) < 0) return false;
    if (_ref->property(&_prop) < 0) _prop.name = 0;
    return false;
}
bool Property::set(const value &val)
{
    if (!_ref || !_prop.name) return false;
    if (mpt_object_pset(_ref, _prop.name, &val) < 0) return false;
    if (_ref->property(&_prop) < 0) _prop.name = 0;
    return true;
}

// create storage
Object::Object(Object &other) : Item<object>(0), _hash(0)
{
    Reference<object> &ref = *this;
    ref = other.ref();
    Slice<const char> d = other.identifier::data();
    if (identifier::setName(d.base(), d.len())) {
        _hash = mpt_hash(d.base(), d.len());
    }
}
Object::Object(object *from) : Item<object>(from), _hash(0)
{ }
Object::Object(const Reference<object> &from) : Item<object>(0), _hash(0)
{
    Reference<object> ref = from;
    _ref = ref.detach();
}

// clear reference to storage
Object::~Object()
{ }

const Reference<object> &Object::ref()
{ return *this; }

// replace existing store
bool Object::setObject(object *o)
{
    if (!o || o == _ref) return false;
    if (_ref) _ref->unref();
    _ref = o;
    return true;
}
Object & Object::operator =(Reference<object> const & from)
{
    Reference<object> ref(from);
    if (ref && setObject(ref)) ref.detach();
    return *this;
}
Object & Object::operator =(Object & other)
{
    Reference<object> ref(other.ref());
    if (ref && setObject(ref)) ref.detach();

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
    prop.select(name);
    return prop;
}
Property Object::operator [](int pos)
{
    Property prop(ref());
    prop.select(pos);
    return prop;
}

struct objectSetContext
{
    object *obj;
    PropertyHandler check;
    void *cdata;
};

static int objectPropertySet(void *addr, const property *pr)
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
    if ((ret = mpt_object_pset(con->obj, pr->name, &pr->val)) < 0) {
        property tmp = *pr;
        tmp.desc = (ret == -1) ? 0 : "";
        return con->check ? con->check(con->cdata, &tmp) : 3;
    }
    return 0;
}
const node *Object::getProperties(const node *head, PropertyHandler proc, void *pdata) const
{
    if (!_ref) return 0;
    objectSetContext con = { _ref, proc, pdata };
    return mpt_node_foreach(head, objectPropertySet, &con, TraverseNonLeafs);
}
int Object::type()
{
    return _ref ? _ref->type() : 0;
}

__MPT_NAMESPACE_END
