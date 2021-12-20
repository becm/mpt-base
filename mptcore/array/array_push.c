/*!
 * MPT core library
 *   add data to encode array
 */

#include <string.h>
#include <errno.h>

#include <sys/uio.h>

#include "output.h"

#include "array.h"

/*!
 * \ingroup mptArray
 * \brief encode data
 * 
 * Encode and add data to array.
 * 
 * Finished data at begin of array indicated by 'done' size.
 * New data is appended in 'scratch' space.
 * 
 * \param arr   array with encoding information
 * \param len   length of data to append
 * \param data  data to append
 * 
 * \return consumed data size
 */
extern ssize_t mpt_array_push(MPT_STRUCT(encode_array) *arr, size_t len, const void *data)
{
	MPT_STRUCT(buffer) *b;
	ssize_t max, add;
	
	/* fast path for raw data */
	if (!arr->_enc) {
		void *dest;
		if (!len) {
			arr->_state.done += arr->_state.scratch;
			arr->_state.scratch = 0;
			return 0;
		}
		if (!data) {
			return MPT_ERROR(MissingData);
		}
		if ((b = arr->_d._buf)
		    && b->_content_traits) {
			return MPT_ERROR(BadType);
		}
		max = arr->_state.done + arr->_state.scratch;
		if (!(dest = mpt_array_insert(&arr->_d, max, len))) {
			return MPT_ERROR(MissingBuffer);
		}
		b = arr->_d._buf;
		memcpy(dest, data, len);
		arr->_state.scratch += len;
		b->_used = max + len;
		
		return len;
	}
	max = arr->_state.done + arr->_state.scratch;
	add = len > 64 ? len : 64;
	
	/* current buffer data */
	if (!(b = arr->_d._buf)) {
		if (max) {
			return MPT_ERROR(BadArgument);
		}
		/* use initial data size */
		if (!(b = _mpt_buffer_alloc(add, 0))) {
			return MPT_ERROR(BadOperation);
		}
		arr->_d._buf = b;
	}
	else if (b->_content_traits) {
		return MPT_ERROR(BadType);
	}
	else if (!(b = b->_vptr->detach(b, max + add))) {
		return MPT_ERROR(BadOperation);
	}
	else {
		arr->_d._buf = b;
	}
	max = 0;
	
	while (1) {
		struct iovec dest;
		ssize_t cont, off;
		
		off = arr->_state.done + arr->_state.scratch;
		
		if ((off = b->_used - off) < 0) {
			return MPT_ERROR(BadArgument);
		}
		dest.iov_base = ((uint8_t *) (b + 1)) + off;
		dest.iov_len  = b->_size - off;
		
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
		if (b->_size < (size_t) off) {
			mpt_log(0, __func__, MPT_LOG(Fatal), "%s: %zi > %zu",
			        MPT_tr("invalid encoder data size"), off, (size_t) b->_size);
			arr->_enc(&arr->_state, 0, 0);
			return MPT_ERROR(BadEncoding);
		}
		b->_used = off;
		
		/* require larger buffer */
		if (cont == MPT_ERROR(MissingBuffer)) {
			if (!(b = b->_vptr->detach(b, b->_size + 64))) {
				return max ? max : MPT_ERROR(MissingBuffer);
			}
			arr->_d._buf = b;
			continue;
		}
		if (cont < 0) {
			return max ? max : cont;
		}
		/* positive no error except for incomplete write */
		if (!data || !len) {
			return max + cont;
		}
		/* space error */
		if (len < (size_t) cont) {
			mpt_log(0, __func__, MPT_LOG(Critical), "%s: %zi > %zu",
			        MPT_tr("bad encoder return size"), cont, len);
			arr->_enc(&arr->_state, 0, 0);
			return MPT_ERROR(BadEncoding);
		}
		/* all data processed */
		if (!(len -= cont)) {
			return max + cont;
		}
		/* advance source data offset */
		data = ((uint8_t *) data) + cont;
		max += cont;
	}
}
