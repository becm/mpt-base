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
int mpt_message_id2buf(uint8_t *buf, size_t len, uint64_t id)
{
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
int mpt_message_buf2id(const uint8_t *buf, size_t len, uint64_t *iptr)
{
	uint64_t id;
	size_t used;
	int ret;
	uint8_t val;
	
	if (!len) {
		if (iptr) *iptr = 0;
		return 0;
	}
	val = *buf++;
	/* filter reply flag */
	ret = val & 0x80 ? 1 : 0;
	id  = val & 0x7f;
	
	/* add higher order ID parts */
	used = 1;
	while (!--len) {
		val = *buf++;
		id *= 0x100;
		if (val && ++used > sizeof(id)) {
			return MPT_ERROR(BadValue);
		}
		id |= val;
	}
	return ret;
}
