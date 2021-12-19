/*!
 * bit field operations
 */
#include "array.h"

/*!
 * \ingroup mptArray
 * \brief set bit
 * 
 * Set bit in map on specified position.
 * 
 * \param base  bitmap base address
 * \param len   number of bytes for map
 * \param pos   position of bit to modify
 * 
 * \retval <0  position out of range
 * \retval 0   bit already set
 * \retval >0  bit modified
 */
extern int mpt_bitmap_set(uint8_t *base, size_t len, long pos)
{
	size_t off;
	
	if (pos < 0 || (off = pos / 8) >= len) {
		return MPT_ERROR(MissingBuffer);
	}
	pos = pos % 8;
	
	if (base[off] & 1<<pos) return 0;
	base[off] |= 1<<pos;
	
	return 1;
}
/*!
 * \ingroup mptArray
 * \brief unset bit
 * 
 * Unset bit in map on specified position.
 * 
 * \param base  bitmap base address
 * \param len   number of bytes for map
 * \param pos   position of bit to modify
 * 
 * \retval <0  position out of range
 * \retval 0   bit already not set
 * \retval >0  bit modified
 */
extern int mpt_bitmap_unset(uint8_t *base, size_t len, long pos)
{
	size_t off;
	
	if (pos < 0 || (off = pos / 8) >= len) {
		return MPT_ERROR(MissingBuffer);
	}
	pos = pos % 8;
	
	if (!(base[off] & 1<<pos)) return 0;
	base[off] &= ~(1<<pos);
	
	return 1;
}
/*!
 * \ingroup mptArray
 * \brief get bit
 * 
 * Get bit value in map on specified position.
 * 
 * \param base  bitmap base address
 * \param len   number of bytes for map
 * \param pos   position of bit to modify
 * 
 * \retval <0  position out of range
 * \retval 0   bit is unset
 * \retval 1   bit is set
 */
extern int mpt_bitmap_get(const uint8_t *base, size_t len, long pos)
{
	size_t off;
	
	if (pos < 0 || (off = pos / 8) >= len) {
		return MPT_ERROR(MissingBuffer);
	}
	pos = pos % 8;
	
	return (base[off] & 1<<pos) ? 1 : 0;
}
