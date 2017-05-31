/*!
 * \file
 * control handler for buffer storage.
 */

#include "message.h"

#include "array.h"

/*!
 * \ingroup mptArray
 * \brief create message metatype
 * 
 * Create metatype with message arguments as (basic) data.
 * 
 * \param ptr   message data
 * \param asep  argument separator
 * 
 * \return pointer to metatype interface
 */
extern MPT_INTERFACE(iterator) *mpt_meta_message(const MPT_STRUCT(message) *ptr, int asep)
{
	MPT_STRUCT(array) a = MPT_ARRAY_INIT;
	MPT_INTERFACE(iterator) *it;
	int len;
	
	if (!ptr) {
		return 0;
	}
	if ((len = mpt_array_message(&a, ptr, asep)) < 0) {
		return 0;
	}
	it = mpt_meta_buffer(&a);
	mpt_array_clone(&a, 0);
	
	return it;
}
