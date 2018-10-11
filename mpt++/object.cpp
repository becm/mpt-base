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
	if (_ref.property(&_prop) < 0) {
		return false;
	}
	_pos = pos;
	return true;
}
bool object::iterator::select(uintptr_t pos)
{
	::mpt::property pr(pos);
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
	if (ret >= 0) {
		return true;
	}
	if (!out) {
		return false;
	}
	::mpt::property pr("");
	if (property(&pr) < 0) {
		pr.name = "object";
	}
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
struct property_error_out { object *obj; logger *out; };
static int object_set_property(void *addr, const property *pr)
{
	struct property_error_out *dat = static_cast<property_error_out *>(addr);
	if (dat->obj->set(pr->name, pr->val, dat->out)) {
		return 0;
	}
	return dat->out ? 1 : -1;
}
bool object::set(const object &from, logger *out)
{
	property_error_out dat = { this, out };
	return mpt_object_foreach(&from, object_set_property, &dat);
}

// class extension for basic property
object::attribute::attribute(object &obj) : _obj(obj)
{ }
// assignment operation for metatypes
object::attribute & object::attribute::operator= (const metatype &meta)
{
	if (_prop.name && !set(meta)) {
		_prop.name = 0;
	}
	return *this;
}
object::attribute & object::attribute::operator= (const char *val)
{
	if (_prop.name && mpt_object_set_string(&_obj, _prop.name, val, 0) < 0) {
		_prop.name = 0;
	}
	return *this;
}

bool object::attribute::select(const char *name)
{
	::mpt::property pr(name);
	if (_obj.property(&pr) < 0) {
		return false;
	}
	_prop = pr;
	return true;
}
bool object::attribute::select(int pos)
{
	if (pos < 0) {
		return false;
	}
	::mpt::property pr(pos);
	if (_obj.property(&pr) < 0) {
		return false;
	}
	_prop = pr;
	return true;
}
bool object::attribute::set(const metatype &src)
{
	if (!_prop.name) {
		return false;
	}
	if (_obj.set_property(_prop.name, &src) < 0) {
		return false;
	}
	if (_obj.property(&_prop) < 0) _prop.name = 0;
	return false;
}
bool object::attribute::set(const value &val)
{
    if (!_prop.name) {
	    return false;
    }
    ::mpt::value tmp = val;
    if (mpt_object_set_value(&_obj, _prop.name, &tmp) < 0) {
	    return false;
    }
    if (_obj.property(&_prop) < 0) _prop.name = 0;
    return true;
}
// get property by name/position
object::attribute object::operator [](const char *name)
{
	object::attribute prop(*this);
	prop.select(name);
	return prop;
}
object::attribute object::operator [](int pos)
{
	object::attribute prop(*this);
	prop.select(pos);
	return prop;
}
const node *object::set(const node *head, property_handler_t proc, void *pdata)
{
	const int traverse = TraverseAll | TraverseChange;
	if (!head) {
		return 0;
	}
	do {
		int ret = mpt_object_set_property(this, traverse, &head->ident, head->meta().instance());
		if (ret) {
			return head;
		}
		if (!proc) {
			continue;
		}
		::mpt::property pr(head->ident.name());
		property(&pr);
		const metatype *mt;
		static const uint8_t metafmt[] = { TypeMetaRef, 0 };
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
