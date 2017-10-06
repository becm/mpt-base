/*!
 * \file
 * control handler for buffer storage.
 */

#include "meta.h"
#include "message.h"

#include "array.h"

/*!
 * \ingroup mptArray
 * \brief create message iterator
 * 
 * Create iterator with message arguments as (basic) data.
 * 
 * \param ptr   message data
 * \param asep  argument separator
 * 
 * \return pointer to iterator interface for message elements
 */
extern MPT_INTERFACE(iterator) *mpt_message_iterator(const MPT_STRUCT(message) *ptr, int asep)
{
	MPT_STRUCT(array) a = MPT_ARRAY_INIT;
	MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(iterator) *it;
	int len;
	
	if (!ptr) {
		return 0;
	}
	if ((len = mpt_array_message(&a, ptr, asep)) < 0) {
		return 0;
	}
	if (!(mt = mpt_meta_buffer(&a))) {
		it = 0;
	}
	else if ((mt->_vptr->conv(mt, MPT_ENUM(TypeIterator), &it)) < 0
	      || !it) {
		mt->_vptr->ref.unref((void *) mt);
	}
	mpt_array_clone(&a, 0);
	
	return it;
}
