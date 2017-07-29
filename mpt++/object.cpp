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

// object assignment
bool object::set(const char *name, const value &val, logger *out)
{
    static const char _fname[] = "mpt::object::set";
    const char *str;
    int ret;
    if (!(str = val.string())) {
        value tmp = val;
        ret = mpt_object_set_value(this, name, &tmp);
    } else {
        ret = mpt_object_set_string(this, name, str, 0);
    }
    if (ret >= 0) return true;
    if (!out) return false;
    struct property pr("");
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
object::Property::Property(const Reference<object> &from) : Reference<object>(from)
{ }
object::Property::Property(object *obj) : Reference<object>(obj)
{ }
object::Property::~Property()
{ }
// assignment operation for metatypes
object::Property & object::Property::operator= (const metatype &meta)
{
    if (_ref && _prop.name && !set(meta)) {
        _prop.name = 0;
    }
    return *this;
}
object::Property & object::Property::operator= (const char *val)
{
    if (_ref && _prop.name && mpt_object_set_string(_ref, _prop.name, val, 0) < 0) {
        _prop.name = 0;
    }
    return *this;
}

bool object::Property::select(const char *name)
{
    if (!_ref) return false;
    struct property pr(name);
    if (_ref->property(&pr) < 0) return false;
    _prop = pr;
    return true;
}
bool object::Property::select(int pos)
{
    if (!_ref || pos <= 0) return false;
    struct property pr(pos);
    if (_ref->property(&pr) < 0) return false;
    _prop = pr;
    return true;
}
bool object::Property::set(const metatype &src)
{
    if (!_ref || !_prop.name) return false;
    if (_ref->setProperty(_prop.name, &src) < 0) return false;
    if (_ref->property(&_prop) < 0) _prop.name = 0;
    return false;
}
bool object::Property::set(const value &val)
{
    if (!_ref || !_prop.name) return false;
    struct value tmp = val;
    if (mpt_object_set_value(_ref, _prop.name, &tmp) < 0) return false;
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
object::Property Object::operator [](const char *name)
{
    object::Property prop(ref());
    prop.select(name);
    return prop;
}
object::Property Object::operator [](int pos)
{
    object::Property prop(ref());
    prop.select(pos);
    return prop;
}
const node *Object::getProperties(const node *head, PropertyHandler proc, void *pdata) const
{
    if (!_ref || !head) return 0;
    do {
        if (mpt_object_set_node(_ref, head, TraverseNonLeafs) < 0) {
            return head;
        }
        if (!proc) {
            return head;
        }
        property pr(head->ident.name());
        _ref->property(&pr);
        const metatype *mt;
        static const char metafmt[] = { mt->Type };
        pr.val.fmt = metafmt;
        pr.val.ptr = &mt;
        if ((mt = head->_meta)) {
           if (mt->conv(pr.val.Type, &pr.val) < 0
            && mt->conv('s', &pr.val.ptr) >= 0) {
                pr.val.fmt = 0;
           }
        }
        if (proc(pdata, &pr) < 0) {
            return head;
        }
    } while ((head = head->next));
    return 0;
}
int Object::type()
{
    return _ref ? _ref->type() : 0;
}

__MPT_NAMESPACE_END
