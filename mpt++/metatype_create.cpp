/*
 * MPT C++ metatype creator
 */

#include <limits>

#include <errno.h>

#include "meta.h"
#include "convert.h"

#include "io.h"

__MPT_NAMESPACE_BEGIN

/*!
 * \ingroup mptMeta
 * \brief create metatype
 * 
 * Create and initialize generic, buffer or special metatype.
 * 
 * \param val  initial metatype value
 * 
 * \return new metatype
 */
metatype *metatype::create(const value &val)
{
	const void *ptr = val.data();
	const char *src;
	int type = val.type_id();
	size_t len;
	
	// generic text data
	if (type == 's') {
		src = val.string();
		len = src ? strlen(src) : 0;
	}
	// extended text format
	else if (!(src = mpt_data_tostring(&ptr, type, &len))) {
		// dispatch to typed metatype creator
		return generic_instance::create(type, ptr);
	}
	// compatible small generic metatype
	if (len < std::numeric_limits<uint8_t>::max()) {
		metatype *m = metatype::basic_instance::create(src, len);
		if (m) {
			return m;
		}
	}
	// create buffer-backed text metatype
	io::buffer::metatype *b;
	if ((b = mpt::io::buffer::metatype::create(0))) {
		if (b->push(len, src) < 0) {
			b->unref();
			return 0;
		}
		b->push(1, "\0"); // terminate string
		b->push(0, 0);    // mark content finished
	}
	return b;
}

__MPT_NAMESPACE_END
