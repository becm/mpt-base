/*!
 * set reply data.
 */

#include <string.h>

#include "event.h"

/*!
 * \ingroup mptEvent
 * \brief event data
 * 
 * Set reply data to new value.
 * 
 * \param rd   reply data data
 * \param len  data size
 * \param val  persistent data
 * 
 * \return state of reply data
 */
extern int mpt_reply_set(MPT_INTERFACE(reply_data) *rd, size_t len, const void *data)
{
	if (len > rd->_max) {
		return MPT_ERROR(BadValue);
	}
	if (data) {
		memcpy(rd->val, data, len);
	} else {
		memset(rd->val, 0, len);
	}
	rd->len = len;
	
	return rd->ptr ? 1 : 0;
}
