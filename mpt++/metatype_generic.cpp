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

metatype::generic::generic(const type_traits *traits, int type) :
	_val(0),
	_traits(traits)
{
	_valtype = type > 0 ? type : 0;
}

metatype::generic::~generic()
{
	if (_traits && _traits->fini) {
		_traits->fini(_val);
	}
}

int metatype::generic::convert(int type, void *ptr)
{
	if (!type) {
		metatype::convert(type, ptr);
		return _valtype ? _valtype : mpt::type_properties< ::mpt::value>::id(true);
	}
	if (type == TypeMetaPtr) {
		if (ptr) {
			*static_cast<metatype **>(ptr) = this;
		}
		return _valtype ? _valtype : static_cast<int>(TypeMetaPtr);
	}
	int me = mpt::type_properties<generic *>::id(true);
	if (me > 0 && type == me) {
		if (ptr) {
			*static_cast<generic **>(ptr) = this;
		}
		return _valtype ? _valtype : static_cast<int>(TypeMetaPtr);
	}
	data_converter_t conv = mpt_data_converter(_valtype);
	if (conv && (type = conv(_val, type, ptr)) >= 0) {
		return type;
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
		if (_traits && _traits->fini) {
			_traits->fini(_val);
		}
		delete this;
	}
}

metatype::generic *metatype::generic::clone() const
{
	return create(_valtype, _traits, _val);
}

metatype::generic *metatype::generic::create(int type, const type_traits *traits, const void *ptr)
{
	if (!traits || !traits->size) {
		errno = EINVAL;
		return 0;
	}
	metatype::generic *mt;
	void *data = malloc(sizeof(*mt) + traits->size);
	if (!data) {
		return 0;
	}
	void *val = static_cast<metatype::generic *>(data) + 1;
	if (!traits->init) {
		if (ptr) {
			memcpy(val, ptr, traits->size);
		}
		else {
			memset(val, 0, traits->size);
		}
	}
	else if (traits->init(val, ptr) < 0) {
		free(data);
		return 0;
	}
	
	mt = new (data) metatype::generic(traits, type);
	mt->_val = val;
	
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
metatype::generic *metatype::generic::create(int type, const void *ptr)
{
	return create(type, type_traits::get(type), ptr);
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
const struct named_traits *metatype::generic::traits(bool obtain)
{
	static const struct named_traits *traits = 0;
	if (traits || !obtain) {
		return traits;
	}
	return traits = type_traits::add_metatype("generic");
}

__MPT_NAMESPACE_END
