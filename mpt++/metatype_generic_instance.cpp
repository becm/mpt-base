/*
 * MPT C++ metatype creator
 */

#include <stdlib.h>
#include <errno.h>
#include <cstring>

#include "types.h"

#include "meta.h"

__MPT_NAMESPACE_BEGIN

metatype::generic_instance::generic_instance(const type_traits *traits, int type) :
	_val(0),
	_traits(traits)
{
	_valtype = type > 0 ? type : 0;
}

metatype::generic_instance::~generic_instance()
{
	if (_traits && _traits->fini) {
		_traits->fini(_val);
	}
}

int metatype::generic_instance::convert(int type, void *ptr)
{
	if (!type) {
		if (ptr) {
			static const uint8_t desc[] = { TypeConvertablePtr, 0 };
			ptr = const_cast<uint8_t *>(desc);
		}
		return TypeMetaPtr;
	}
	if (type == TypeMetaPtr) {
		if (ptr) {
			*static_cast<metatype **>(ptr) = this;
		}
		return _valtype ? _valtype : static_cast<int>(TypeMetaPtr);
	}
	if (type == static_cast<int>(_valtype)) {
		// TODO value conversion
	}
	return BadType;
}

uintptr_t metatype::generic_instance::addref()
{
	return _ref.raise();
}

void metatype::generic_instance::unref()
{
	if (!_ref.lower()) {
		delete this;
		free(this);
	}
}

metatype::generic_instance *metatype::generic_instance::clone() const
{
	return create(_valtype, _traits, _val);
}

metatype::generic_instance *metatype::generic_instance::create(int type, const type_traits *traits, const void *ptr)
{
	if (!traits || !traits->size) {
		errno = EINVAL;
		return 0;
	}
	metatype::generic_instance *mt;
	void *data = malloc(sizeof(*mt) + traits->size);
	if (!data) {
		return 0;
	}
	mt = static_cast<metatype::generic_instance *>(data);
	if (traits->init) {
		if (ptr) {
			memcpy((void *) (mt + 1), ptr, traits->size);
		}
		else {
			memset((void *) (mt + 1), 0, traits->size);
		}
	}
	else if (traits->init(mt + 1, ptr) < 0) {
		free(data);
		return 0;
	}
	
	mt = new (data) metatype::generic_instance(traits, type);
	mt->_val = mt + 1;
	
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
metatype::generic_instance *metatype::generic_instance::create(int type, const void *ptr)
{
	return create(type, type_traits::get(type), ptr);
}

__MPT_NAMESPACE_END
