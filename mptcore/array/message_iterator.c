/*!
 * \file
 * control handler for buffer storage.
 */

#include "message.h"

#include "array.h"

/*!
 * \ingroup mptArray
 * \brief create message iterator
 * 
 * Create iterator with message text segments.
 * 
 * \param ptr   message data
 * \param asep  argument separator
 * 
 * \return new iterator for message elements
 */
extern MPT_INTERFACE(metatype) *mpt_message_iterator(const MPT_STRUCT(message) *ptr, int asep)
{
	MPT_STRUCT(array) a = MPT_ARRAY_INIT;
	MPT_INTERFACE(metatype) *mt;
	int len;
	
	if (ptr && (len = mpt_array_message(&a, ptr, asep)) < 0) {
		return 0;
	}
	mt = mpt_meta_buffer(&a);
	mpt_array_clone(&a, 0);
	return mt;
}
