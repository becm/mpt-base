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
extern ssize_t mpt_array_push(MPT_STRUCT(array) *arr, MPT_STRUCT(codestate) *info, MPT_TYPE(DataEncoder) enc, const struct iovec *data)
{
	MPT_STRUCT(buffer) *b;
	struct iovec src;
	size_t len, max;
	
	
	/* fast path for raw data */
	if (!enc) {
		if (!data) {
			return 0;
		}
		if (!(len = data->iov_len)) {
			return -1;
		}
		if (!data->iov_base) {
			return -2;
		}
		if (!mpt_array_append(arr, data->iov_len, data->iov_base)) {
			return -1;
		}
		if (info) {
			info->scratch += len;
		}
		return len;
	}
	max = 0;
	if (data) {
		src = *data;
		data = &src;
		max = data->iov_len;
	} else {
		src.iov_len = 0;
	}
	/* current buffer data */
	if (!(b = arr->_buf)) {
		/* use initial data size */
		if (!(b = _mpt_buffer_realloc(0, max > 64 ? max : 64))) {
			return -1;
		}
		arr->_buf = b;
	}
	len = info->done + info->scratch;
	
	while (1) {
		struct iovec dest;
		ssize_t cont, off;
		
		if ((off = b->used - len) < 0) {
			errno = EINVAL;
			return -1;
		}
		
		dest.iov_base = ((uint8_t *) (b+1)) + off;
		dest.iov_len  = b->size - off;
		
		/* next regular operation */
		cont = enc(info, &dest, data);
		len  = info->done + info->scratch;
		off += len;
		
		/* size update */
		if (b->size < len) {
			MPT_ABORT("invalid encoder data size");
			enc(info, 0, 0);
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
		else if (!data || !src.iov_len || !src.iov_base) {
			return max;
		}
		/* space error */
		else if (src.iov_len < (size_t) cont) {
			MPT_ABORT("bad encoder return size");
			enc(info, 0, 0);
			return -1;
		}
		/* all data processed */
		else if (!(src.iov_len -= cont)) {
			return max;
		}
		/* advance source data offset */
		else {
			src.iov_base = ((uint8_t *) src.iov_base) + cont;
		}
		
		/* get larger buffer */
		if (!b->resize || !(b = b->resize(b, b->size + 64))) {
			cont = max - src.iov_len;
			return cont ? cont : -2;
		}
		arr->_buf = b;
	}
}
