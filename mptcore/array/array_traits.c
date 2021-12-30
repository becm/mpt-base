/*!
 * register array with type system.
 */

#include "types.h"

#include "array.h"

static void _array_fini(void *ptr)
{
	MPT_STRUCT(array) *arr = ptr;
	MPT_STRUCT(buffer) *buf = arr->_buf;
	if (buf) {
		buf->_vptr->unref(buf);
		arr->_buf = 0;
	}
}
static int _array_init(void *ptr, const void *src)
{
	MPT_STRUCT(array) *arr = ptr;
	MPT_STRUCT(buffer) *buf = 0;
	if (src && (buf = ((const MPT_STRUCT(array) *) src)->_buf)) {
		if (!buf->_vptr->addref(buf)) {
			return MPT_ERROR(BadOperation);
		}
	}
	arr->_buf = buf;
	return buf ? 1 : 0;
}

/*!
 * \ingroup mptArray
 * \brief get array traits
 * 
 * Get array type operations and size.
 * 
 * \return array type traits
 */
extern const MPT_STRUCT(type_traits) *mpt_array_traits()
{
	static const MPT_STRUCT(type_traits) traits = {
		_array_init,
		_array_fini,
		sizeof(MPT_STRUCT(array))
	};
	return &traits;
}
