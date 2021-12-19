/*!
 * MPT core library
 *   assign array buffer reference
 */

#include "array.h"

/*!
 * \ingroup mptArray
 * \brief set buffer reference
 * 
 * Copy buffer reference with source array.
 * Pass source zero pointer to clear target buffer reference.
 * 
 * \param arr  target array pointer
 * \param from source array pointer
 * 
 * \retval mpt::BadType       content type mismatch
 * \retval mpt::BadOperation  unable to create clone
 * \retval 0  no change
 * \retval 1  new buffer assigned
 * \retval 2  buffer removed
 * \retval 3  existing buffer replaced
 */
int mpt_array_clone(MPT_STRUCT(array) *arr, const MPT_STRUCT(array) *from)
{
	MPT_STRUCT(buffer) *buf = arr->_buf, *set = 0;
	
	if (from) {
		set = from->_buf;
		/* no action for identical content */
		if (set == buf) {
			return 0;
		}
		/* buffers content types must be identical */
		if (set && buf && (set->_content_traits != buf->_content_traits)) {
			return MPT_ERROR(BadType);
		}
		/* increase buffer refcount */
		if (!set->_vptr->addref(set)) {
			return MPT_ERROR(BadOperation);
		}
	}
	/* replace array content */
	arr->_buf = set;
	/* remove old buffer reference */
	if (buf) {
		buf->_vptr->unref(buf);
		return set ? 3 : 2;
	}
	return set ? 1 : 0;
}
