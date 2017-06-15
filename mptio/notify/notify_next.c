/*!
 * get waiting element from poll descriptor.
 */

#include <errno.h>

#include "array.h"
#include "event.h"

#include "notify.h"

/*!
 * \ingroup mptNotify
 * \brief next input
 * 
 * Next active input of notification descriptor.
 * 
 * \param no	notification descriptor
 * 
 * \return next command id
 */
extern MPT_INTERFACE(input) *mpt_notify_next(const MPT_STRUCT(notify) *no)
{
	MPT_STRUCT(buffer) *s;
	size_t len;
	
	if ((s = no->_wait._buf) && (len = s->_used / sizeof(void *))) {
		void **first = (void *) (s+1);
		size_t i;
		
		for (i = 0; i < len; i++) {
			MPT_INTERFACE(input) *curr;
			if (!(curr = first[i])) continue;
			first[i] = 0;
			return curr;
		}
		s->_used = 0;
	}
	return 0;
}
