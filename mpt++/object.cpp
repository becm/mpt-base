/*
 * generic metatype Object wrapper and property extension
 */

#include "node.h"
#include "convert.h"

#include "object.h"

__MPT_NAMESPACE_BEGIN

template<> int Item<object>::type()
{
    static int id = 0;
    if (!id) {
        id = makeItemId(object::Type);
    }
    return id;
}

bool object::const_iterator::select(uintptr_t pos)
{
    _prop.name = 0;
    _prop.desc = (const char *) pos;
    if (_ref.property(&_prop) < 0) {
        return false;
    }
    _pos = pos;
    return true;
}
bool object::iterator::select(uintptr_t pos)
{
    struct property pr(pos);
    if (_ref.property(&pr) < 0) {
        return false;
    }
    _name = pr.name;
    _pos = pos;
    return true;
}

int object::type() const
{
    struct property pr("");
    return property(&pr);
}
// object assignment
bool object::set(const char *name, const value &val, logger *out)
{
    static const char _fname[] = "mpt::object::set";
    const char *str;
    int ret;
    if (!(str = val.string())) {
        value tmp = val;
        ret = mpt_object_iset(this, name, &tmp);
    } else {
        ret = mpt_object_pset(this, name, str, 0);
    }
    if (ret >= 0) return true;
    if (!out) return false;
    struct property pr;
    if (property(&pr) < 0) pr.name = "object";
    pr.val.fmt = val.fmt;
    if (!(pr.val.ptr = val.ptr)) {
        pr.val.fmt = 0;
        pr.val.ptr = "";
    }

    const char *err;
    if (ret == BadArgument) {
        err = MPT_tr("bad property");
        if (name) {
            out->message(_fname, out->Error, "%s: %s.%s", err, pr.name, name);
        } else {
            out->message(_fname, out->Error, "%s: %s", err, pr.name);
        }
        return false;
    }
    if (ret == BadValue) {
        err = MPT_tr("bad property value");
        if (name) {
            out->message(_fname, out->Error, "%s: %s.%s = \"%s\"", err, pr.name, name, pr.val.ptr);
        } else {
            out->message(_fname, out->Error, "%s: %s = \"%s\"", err, pr.name, pr.val.ptr);
        }
        return false;
    }
    if (ret == BadType) {
        err = MPT_tr("bad property type");
        if (name) {
            out->message(_fname, out->Error, "%s: %s.%s = <%s>", err, pr.name, name, pr.val.fmt);
        } else {
            out->message(_fname, out->Error, "%s: %s = <%s>", err, pr.name, pr.val.fmt);
        }
        return false;
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
    if (_ref && _prop.name && !set(val)) {
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
    value tmp = val;
    if (mpt_object_iset(_ref, _prop.name, &tmp) < 0) return false;
    if (_ref->property(&_prop) < 0) _prop.name = 0;
    return true;
}

// create storage
Object::Object(Object &other) : Item<object>(0), _hash(0)
{
    Reference<object>::operator=(other.ref());
    if (identifier::setName(other)) {
        Slice<const char> d = nameData();
        _hash = mpt_hash(d.base(), d.length());
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
bool Object::setPointer(object *o)
{
    if (!o || o == _ref) return false;
    if (_ref) _ref->unref();
    _ref = o;
    return true;
}
Object & Object::operator =(Reference<object> const & from)
{
    Reference<object> ref(from);
    object *obj = ref.pointer();
    if (obj && setPointer(obj)) ref.detach();
    return *this;
}
Object & Object::operator =(Object & other)
{
    Reference<object> ref(other.ref());
    object *obj = ref.pointer();
    if (obj && setPointer(obj)) ref.detach();

    if (identifier::setName(other)) {
        Slice<const char> d = nameData();
        _hash = mpt_hash(d.base(), d.length());
    }
    return *this;
}

// set identifier
bool Object::setName(const char *name, int len)
{
    if (!identifier::setName(name, len)) {
        return false;
    }
    Slice<const char> d = nameData();

    _hash = mpt_hash(d.base(), d.length());

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
    const objectSetContext *con = static_cast<objectSetContext *>(addr);
    int ret;
    if (!pr || !pr->name) {
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
    const char *val;
    if ((val = pr->val.string())) {
        ret = mpt_object_pset(con->obj, pr->name, val, 0);
    } else {
        value tmp = pr->val;
        ret = mpt_object_iset(con->obj, pr->name, &tmp);
    }
    if (ret < 0) {
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
