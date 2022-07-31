/*
 * MPT C++ convertable operations
 */

#include <cstdio>

#include "layout.h"

__MPT_NAMESPACE_BEGIN

/*!
 * \ingroup mptConvert
 * \brief get string data
 * 
 * Convert value to string data
 * 
 * \return C string base pointer
 */
template <> int type_properties<color>::id(bool)
{
	return mpt_color_typeid();
}

__MPT_NAMESPACE_END

std::ostream &operator<<(std::ostream &o, const mpt::color &c)
{
	char buf[256];
	int adv;
	/* represent color data */
	if (c.alpha != 0xff) {
		adv = snprintf(buf, sizeof(buf), "#%02x%02x%02x%02x", c.red, c.green, c.blue, c.alpha);
	} else {
		adv = snprintf(buf, sizeof(buf), "#%02x%02x%02x", c.red, c.green, c.blue);
	}
	if (adv <= 0) {
		return o;
	}
	return o << buf;
}
