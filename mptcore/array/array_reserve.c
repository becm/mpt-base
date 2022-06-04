/*!
 * MPT core library
 *   reserve typed data on array
 */

#include <errno.h>

#include "types.h"

#include "array.h"

/*!
 * \ingroup mptCore
 * \brief reserve data on array
 * 
 * Prepare elements on array.
 * Change buffer content type or create new instance if required.
 * 
 * \param arr     array to be updated
 * \param len     number of bytes to reserve
 * \param traits  type traits for elements
 * 
 * \return buffer satisfying requested type and space
 */
extern MPT_STRUCT(buffer) *mpt_array_reserve(MPT_STRUCT(array) *arr, size_t len, const MPT_STRUCT(type_traits) *traits)
{
	const MPT_STRUCT(type_traits) *old = 0;
	MPT_STRUCT(buffer) *buf;
	int flags = -1;
	
	/* check arguments */
	if (traits) {
		size_t align;
		if (!traits->size) {
			errno = EINVAL;
			return 0;
		}
		/* total data must align with traits size */
		if ((align = len % traits->size)) {
			len += traits->size - align;
		}
	}
	/* check compatibility of existing data */
	if ((buf = arr->_buf)) {
		flags = buf->_vptr->get_flags(buf);
		old = buf->_content_traits;
	}
	/* distinct instance is require */
	if ((flags & MPT_ENUM(BufferShared))
	 || (flags & MPT_ENUM(BufferImmutable))) {
		MPT_STRUCT(buffer) *reserve;
		
		if (!(reserve = _mpt_buffer_alloc(len, 0))) {
			return 0;
		}
		reserve->_content_traits = traits;
		if (buf) {
			size_t used = buf->_used;
			if (old) {
				used -= used % old->size;
			}
			/* copy compatible content */
			if ((old == traits)
			 && !(flags & MPT_ENUM(BufferNoCopy))) {
				if (used > len) {
					used = len;
				}
				if (used && !mpt_buffer_set(reserve, traits, used, buf + 1, 0)) {
					reserve->_vptr->unref(reserve);
					return 0;
				}
			}
			buf->_vptr->unref(buf);
		}
		arr->_buf = reserve;
		return reserve;
	}
	/* clear incompatible data on non-shared buffer */
	if ((old != traits)) {
		void (*fini)(void *) = 0;
		if (!old || !(fini = old->fini) || !traits || (fini != traits->fini)) {
			if (fini) {
				size_t pos, used = buf->_used, size = old->size;
				used -= used % size;
				uint8_t *ptr = (void *) (buf + 1);
				for (pos = 0; pos < used; pos += size) {
					fini(ptr + pos);
				}
			}
			buf->_used = 0;
		}
	}
	/* existing data can be reused */
	if (!(buf = buf->_vptr->detach(buf, len))) {
		return buf;
	}
	arr->_buf = buf;
	buf->_content_traits = traits;
	
	return buf;
}
