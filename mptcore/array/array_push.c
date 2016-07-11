/*!
 * send (double) world data message via stream.
 */

#include <string.h>
#include <errno.h>

#include <sys/uio.h>

#include "array.h"

/*!
 * \ingroup mptArray
 * \brief append data to array
 * 
 * Add encoded data to array.
 * Encoder state is saved in "unused" part of buffer data.
 * Size of state data must be returned by encoding funtion.
 * 
 * \param arr   array to append data
 * \param enc   encoder plugin function
 * \param len   length of data to append
 * \param data  data to append
 * 
 * \return consumed data size
 */
extern ssize_t mpt_array_push(MPT_STRUCT(encode_array) *arr, size_t len, const void *data)
{
	MPT_STRUCT(buffer) *b;
	size_t max;
	
	/* fast path for raw data */
	if (!arr->_enc) {
		if (!len) {
			arr->_state.done += arr->_state.scratch;
			arr->_state.scratch = 0;
			return 0;
		}
		if (!data) {
			return MPT_ERROR(MissingData);
		}
		if (!mpt_array_append(&arr->_d, len, data)) {
			return MPT_ERROR(MissingBuffer);
		}
		arr->_state.scratch += len;
		
		return len;
	}
	/* current buffer data */
	if (!(b = arr->_d._buf)) {
		/* use initial data size */
		if (!(b = _mpt_buffer_realloc(0, len > 64 ? len : 64))) {
			return -1;
		}
		arr->_d._buf = b;
	}
	max = 0;
	
	while (1) {
		struct iovec dest;
		ssize_t cont, off;
		
		off = arr->_state.done + arr->_state.scratch;
		
		if ((off = b->used - off) < 0) {
			errno = EINVAL;
			return -1;
		}
		dest.iov_base = ((uint8_t *) (b+1)) + off;
		dest.iov_len  = b->size - off;
		
		/* next regular operation */
		if (!len) {
			cont = arr->_enc(&arr->_state, &dest, 0);
		} else {
			struct iovec src;
			src.iov_base = (void *) data;
			src.iov_len  = len;
			cont = arr->_enc(&arr->_state, &dest, &src);
		}
		off += arr->_state.done + arr->_state.scratch;
		
		/* size update */
		if (b->size < (size_t) off) {
			MPT_ABORT("invalid encoder data size");
			arr->_enc(&arr->_state, 0, 0);
			return -1;
		}
		b->used = off;
		
		/* retry on size problem only */
		if (cont == MPT_ERROR(MissingBuffer)) {
			;
		}
		else if (cont < 0) {
			return cont;
		}
		/* positive no error except for incomplete write */
		else if (!data || !len) {
			return max + cont;
		}
		/* space error */
		else if (len < (size_t) cont) {
			MPT_ABORT("bad encoder return size");
			arr->_enc(&arr->_state, 0, 0);
			return -1;
		}
		/* all data processed */
		else if (!(len -= cont)) {
			return max + cont;
		}
		/* advance source data offset */
		else {
			data = ((uint8_t *) data) + cont;
		}
		max += cont;
		
		/* get larger buffer */
		if (!b->resize || !(b = b->resize(b, b->size + 64))) {
			cont = len - max;
			return cont ? cont : -2;
		}
		arr->_d._buf = b;
	}
}
