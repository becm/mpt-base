/*!
 * encode and decode message ID
 */

#include "message.h"

/*!
 * \ingroup mptMessage
 * \brief set message ID
 * 
 * Set buffer data to message ID.
 * 
 * \param buf  buffer data
 * \param len  message ID length
 * \param id   message ID to encode
 * 
 * \return result of push operation
 */
int mpt_message_id2buf(uint64_t id, void *ptr, size_t len)
{
	uint8_t *buf = ptr;
	int used;
	if (!len--) {
		return id ? MPT_ERROR(MissingBuffer) : 0;
	}
	buf[len] = 0xff & id;
	used = 1;
	while (len--) {
		id /= 0x100;
		if (id) ++used;
		buf[len] = 0xff & id;
	}
	if (id) {
		return MPT_ERROR(MissingBuffer);
	}
	if ((*buf & 0x80)) {
		return MPT_ERROR(BadValue);
	}
	return used;
}
/*!
 * \ingroup mptMessage
 * \brief set message ID
 * 
 * Set buffer data to message ID.
 * 
 * \param buf  buffer data
 * \param len  message ID length
 * \param id   message ID to encode
 * 
 * \return result of push operation
 */
int mpt_message_buf2id(const void *ptr, size_t len, uint64_t *iptr)
{
	const uint8_t *buf = ptr;
	uint64_t id;
	size_t used;
	uint8_t val;
	
	if (!len) {
		if (iptr) *iptr = 0;
		return 0;
	}
	/* initial value setup */
	id = *buf++;
	used = id ? 1 : 0;
	
	/* add higher order ID parts */
	while (!--len) {
		val = *buf++;
		id *= 0x100;
		if ((val || used) && ++used > sizeof(id)) {
			return MPT_ERROR(BadValue);
		}
		id |= val;
	}
	return used;
}
