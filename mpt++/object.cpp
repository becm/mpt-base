/*
 * generic metatype Object wrapper and property extension
 */

#include "node.h"
#include "object.h"

__MPT_NAMESPACE_BEGIN

// class extension for basic property
Property::Property(const Reference<metatype> &from) : _obj(0)
{
    Reference<metatype> ref = from;
    metatype *m;
    if ((m = ref) && (_obj = m->cast<object>())) {
        ref.detach();
    }
}
Property::Property(object *obj) : _obj(obj)
{ }
Property::~Property()
{
    if (_obj) _obj->unref();
}
// assignment operation for metatypes
Property & Property::operator= (metatype &meta)
{
    if (!_obj) return *this;

    const char *fmt = (char *) meta.typecast(0);
    char type[2];

    while ((*type = *fmt++)) {
        void *data = meta.typecast(*type);
        if (!data) continue;
        value val(type, data);
        if (mpt_object_pset(_obj, _prop.name, &val) >= 0) break;
    }
    return *this;
}

Property & Property::operator= (const char *val)
{
    return *this = value((const char *) 0, (const void *) val);
}

bool Property::select(const char *name)
{
    if (!_obj) return false;
    property pr(name);
    if (_obj->property(&pr) < 0) return false;
    _prop = pr;
    return true;
}
bool Property::select(int pos)
{
    if (!_obj || pos <= 0) return false;
    property pr(pos);
    if (_obj->property(&pr) < 0) return false;
    _prop = pr;
    return true;
}
bool Property::set(source &src)
{
    if (!_obj || !_prop.name) return false;
    if (_obj->setProperty(_prop.name, &src) < 0) return false;
    if (_obj->property(&_prop) < 0) _prop.name = 0;
    return false;
}
bool Property::set(const value &val)
{
    if (!_obj || !_prop.name) return false;
    if (mpt_object_pset(_obj, _prop.name, &val) < 0) return false;
    if (_obj->property(&_prop) < 0) _prop.name = 0;
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

static int metaPropertySet(void *addr, const property *pr)
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
    object *obj;
    if (!_ref || !(obj = _ref->cast<object>())) return 0;
    objectSetContext con = { obj, proc, pdata };
    return mpt_node_foreach(head, metaPropertySet, &con, TraverseNonLeafs);
}
int Object::type()
{
    object *obj;
    if (!_ref || !(obj = _ref->cast<object>())) return 0;
    return obj->type();
}

class globalMetatype : public metatype
{
public:
    globalMetatype()  { }
    virtual ~globalMetatype() { }

    metatype *addref() { return this; }
    int unref() { return 0; }
    int assign(const value *val)
    {
        return val ? mpt::BadOperation : 0;
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
