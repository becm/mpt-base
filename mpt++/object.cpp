/*
 * generic metatype Object wrapper and property extension
 */

#include "node.h"
#include "convert.h"

#include "object.h"

__MPT_NAMESPACE_BEGIN

bool object::const_iterator::select(uintptr_t pos)
{
    _prop.name = 0;
    _prop.desc = (const char *) pos;
    if (_ref.property_get(&_prop) < 0) {
        return false;
    }
    _pos = pos;
    return true;
}
bool object::iterator::select(uintptr_t pos)
{
    property pr(pos);
    if (_ref.property_get(&pr) < 0) {
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
    property pr("");
    if (property_get(&pr) < 0) pr.name = "object";
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
bool object::set(const object &from, logger *out)
{
    propertyErrorOut dat = { this, out };
    return mpt_object_foreach(&from, objectSetProperty, &dat);
}

// class extension for basic property
object::Property::Property(object &obj) : _obj(obj)
{ }
// assignment operation for metatypes
object::Property & object::Property::operator= (const metatype &meta)
{
    if (_prop.name && !set(meta)) {
        _prop.name = 0;
    }
    return *this;
}
object::Property & object::Property::operator= (const char *val)
{
    if (_prop.name && mpt_object_set_string(&_obj, _prop.name, val, 0) < 0) {
        _prop.name = 0;
    }
    return *this;
}

bool object::Property::select(const char *name)
{
    struct property pr(name);
    if (_obj.property_get(&pr) < 0) return false;
    _prop = pr;
    return true;
}
bool object::Property::select(int pos)
{
    if (pos < 0) return false;
    struct property pr(pos);
    if (_obj.property_get(&pr) < 0) return false;
    _prop = pr;
    return true;
}
bool object::Property::set(const metatype &src)
{
    if (!_prop.name) return false;
    if (_obj.property_set(_prop.name, &src) < 0) return false;
    if (_obj.property_get(&_prop) < 0) _prop.name = 0;
    return false;
}
bool object::Property::set(const value &val)
{
    if (!_prop.name) return false;
    struct value tmp = val;
    if (mpt_object_set_value(&_obj, _prop.name, &tmp) < 0) return false;
    if (_obj.property_get(&_prop) < 0) _prop.name = 0;
    return true;
}
// get property by name/position
object::Property object::operator [](const char *name)
{
    object::Property prop(*this);
    prop.select(name);
    return prop;
}
object::Property object::operator [](int pos)
{
    object::Property prop(*this);
    prop.select(pos);
    return prop;
}
const node *object::set(const node *head, PropertyHandler proc, void *pdata)
{
    const int traverse = TraverseAll | TraverseChange;
    if (!head) {
        return 0;
    }
    do {
        int ret = mpt_object_set_property(this, traverse, &head->ident, head->meta().pointer());
        if (ret) {
            return head;
        }
        if (!proc) {
            continue;
        }
        property pr(head->ident.name());
        property_get(&pr);
        const metatype *mt;
        static const uint8_t metafmt[] = { metatype::Type, 0 };
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

__MPT_NAMESPACE_END
