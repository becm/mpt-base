/*!
 * \file
 * control handler for buffer storage.
 */

#include "message.h"
#include "types.h"

#include "array.h"

/*!
 * \ingroup mptArray
 * \brief get message arguments
 * 
 * Set array content to arguments on message data.
 * 
 * \param arr   array data pointer
 * \param ptr   message data
 * \param asep  argument separator
 * 
 * \return number of arguments found
 */
extern int mpt_array_message(MPT_STRUCT(array) *arr, const MPT_STRUCT(message) *ptr, int asep)
{
	MPT_STRUCT(message) msg;
	MPT_STRUCT(array) a = MPT_ARRAY_INIT;
	char *base;
	ssize_t len;
	int narg = 0;
	
	if (!ptr || !(len = mpt_message_length(ptr))) {
		mpt_array_clone(arr, 0);
		return narg;
	}
	/* prepare for maximum size */
	if (!(base = mpt_array_slice(&a, 0, len+1))) {
		return MPT_ERROR(BadOperation);
	}
	a._buf->_used = 0;
	
	msg = *ptr;
	while ((len = mpt_message_argv(&msg, asep)) >= 0) {
		if (!len) {
			if (asep) {
				break;
			}
		}
		else if (!(base = mpt_array_append(&a, len, 0))) {
			mpt_array_clone(&a, 0);
			return MPT_ERROR(MissingBuffer);
		}
		/* save next argument to array */
		else {
			mpt_message_read(&msg, len, base);
		}
		++narg;
		/* save argument separation */
		if (!(base = mpt_array_append(&a, 1, 0))) {
			mpt_array_clone(&a, 0);
			return MPT_ERROR(MissingBuffer);
		}
		/* skip argument separation */
		mpt_message_read(&msg, 1, 0);
	}
	a._buf->_content_traits = mpt_type_traits('c');
	mpt_array_clone(arr, &a);
	mpt_array_clone(&a, 0);
	return narg;
}
