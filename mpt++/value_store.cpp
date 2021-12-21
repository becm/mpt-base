/*
 * MPT C++ typed array
 */

#include "message.h"

#include "values.h"

__MPT_NAMESPACE_BEGIN

template <> const struct type_traits *type_properties<value_store>::traits()
{
	return mpt_value_store_traits();
}

// typed information for array
long value_store::element_count() const
{
	const array::content *d;
	if (!(d = _d.data())) {
		return BadArgument;
	}
	const struct type_traits *t;
	if (!(t = d->content_traits())
	    || !t->size) {
		return BadType;
	}
	return d->length() / t->size;
}
void value_store::set_modified(bool set)
{
	if (set) {
		_flags |= ValueChange;
	} else {
		_flags &= ~ValueChange;
	}
}
void *value_store::reserve(long count, const struct type_traits &traits)
{
	return mpt_array_reserve(&_d, count, &traits);
}
void *value_store::set(const struct type_traits &traits, size_t len, const void *data, long pos)
{
	return mpt_array_set(&_d, &traits, len, data, pos);
}

long maxsize(span<const value_store> sl, const struct type_traits *traits)
{
	const value_store *val = sl.begin();
	long len = -1;
	for (size_t i = 0, max = sl.size(); i < max; ++i) {
		if (traits) {
			const array::content *d = val->data();
			if (!d || (traits != d->content_traits())) {
				continue;
			}
		}
		long curr = val->element_count();
		if (curr < 0) {
			continue;
		}
		if (curr > len) {
			len = curr;
		}
	}
	return len;
}

__MPT_NAMESPACE_END
