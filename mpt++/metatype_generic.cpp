/*
 * MPT C++ metatype creator
 */

#include <stdlib.h>
#include <errno.h>
#include <cstring>

#include "convert.h"
#include "types.h"

#include "meta.h"

__MPT_NAMESPACE_BEGIN

metatype::generic::generic() : _traits(0), _val(0), _type(0)
{ }

metatype::generic::~generic()
{
	if (_val && _traits && _traits->fini) {
		_traits->fini(_val);
	}
	_val = 0;
}

int metatype::generic::convert(value_t type, void *ptr)
{
	int valtype = mpt::type_properties< ::mpt::value>::id(true);
	
	if (!type) {
		metatype::convert(type, ptr);
		return _type ? _type : valtype;
	}
	if (type == TypeMetaPtr) {
		if (ptr) {
			*static_cast<metatype **>(ptr) = this;
		}
		return _type ? _type : static_cast<int>(TypeMetaPtr);
	}
	int me = mpt::type_properties<generic *>::id(true);
	if (me > 0 && type == static_cast<value_t>(me)) {
		if (ptr) {
			*static_cast<generic **>(ptr) = this;
		}
		return _type ? _type : static_cast<int>(TypeMetaPtr);
	}
	if (valtype > 0 && type == static_cast<value_t>(valtype)) {
		if (ptr) {
			static_cast< ::mpt::value *>(ptr)->set(_type, _val);
		}
		return me > 0 ? me : mpt::type_properties< ::mpt::metatype *>::id(true);
	}
	data_converter_t conv = mpt_data_converter(_type);
	int ret;
	if (conv && (ret = conv(_val, type, ptr)) >= 0) {
		return ret;
	}
	return BadType;
}

uintptr_t metatype::generic::addref()
{
	return _ref.raise();
}

void metatype::generic::unref()
{
	if (!_ref.lower()) {
		delete this;
	}
}

metatype::generic *metatype::generic::clone() const
{
	return create(_type, _val, *_traits);
}

metatype::generic *metatype::generic::create(value_t type, const void *ptr, const type_traits &traits)
{
	if (!traits.size) {
		errno = EINVAL;
		return 0;
	}
	metatype::generic *mt;
	void *data = malloc(sizeof(*mt) + traits.size);
	if (!data) {
		return 0;
	}
	void *val = static_cast<metatype::generic *>(data) + 1;
	if (!traits.init) {
		if (ptr) {
			memcpy(val, ptr, traits.size);
		}
		else {
			memset(val, 0, traits.size);
		}
	}
	else if (traits.init(val, ptr) < 0) {
		free(data);
		return 0;
	}
	
	mt = new (data) metatype::generic();
	mt->_traits = &traits;
	mt->_val = val;
	mt->_type = type;
	
	return mt;
}

/*!
 * \ingroup mptType
 * \brief create typed metatype
 * 
 * Create and initialize Metatype class of specific type
 * 
 * \param traits  traits for data pointer
 * \param type    data type ID
 * \param ptr     source data pointer
 * 
 * \return new metatype
 */
metatype::generic *metatype::generic::create(value_t type, const void *ptr)
{
	const type_traits *tt = type_traits::get(type);
	if (!tt) {
		return 0;
	}
	return create(type, ptr, *tt);
}

/*!
 * \ingroup mptMeta
 * \brief query traits for generic metatype
 * 
 * Query and register named traits for specialized metatype pointer.
 * 
 * \param val  initial metatype value
 * 
 * \return new metatype
 */
const struct named_traits *metatype::generic::pointer_traits(bool obtain)
{
	static const struct named_traits *traits = 0;
	if (!traits && obtain && !(traits = type_traits::add_metatype("generic"))) {
		traits = type_traits::add_metatype();
	}
	return traits;
}

__MPT_NAMESPACE_END
