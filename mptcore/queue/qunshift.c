/*!
 * add/remove data on left side end.
 */

#include <string.h>
#include <stdint.h>

#include "queue.h"

/*!
 * \ingroup mptQueue
 * \brief prepend queue data
 * 
 * Add data to queue start.
 * 
 * \param queue fifo data element
 * \param len   size of new data
 * \param data  data to prepend
 * 
 * \return consumed data
 */
extern int mpt_qunshift(MPT_STRUCT(queue) *queue, size_t len, const void *data)
{
	int ret;
	if ((ret = mpt_qpre(queue, len)) < 0) {
		return ret;
	}
	return mpt_queue_set(queue, 0, len, data);
}
