/*
 * MPT C++ convertable operations
 */

#include <cstdio>

#include "values.h"

__MPT_NAMESPACE_BEGIN

template <> int type_properties<point<float> >::id(bool) {
	static int ptype = 0;
	if (!ptype) {
		int type = mpt_fpoint_typeid();
		if (type <= 0) {
			return type ? type : static_cast<int>(BadType);
		}
		ptype = type;
	}
	return ptype;
}
template <> const struct type_traits *type_properties<point<float> >::traits() {
	return type_traits::get(id(true));
}

template <> int type_properties<point<double> >::id(bool) {
	static int ptype = 0;
	if (!ptype) {
		int type = mpt_type_add(traits());
		if (type <= 0) {
			return type ? type : static_cast<int>(BadType);
		}
		ptype = type;
	}
	return ptype;
}
template <> const struct type_traits *type_properties<point<double> >::traits() {
	static const struct type_traits traits(sizeof(point<double>));
	return &traits;
}

__MPT_NAMESPACE_END
