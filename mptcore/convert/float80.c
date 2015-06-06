/*!
 * \file
 * 80bit extended float handling
 */

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief get extended precision float values
 * 
 * Convert shortened to native long double format.
 * 
 * \param len number of elements
 * \param src 10 byte float values
 * \param dst native long double values
 */
extern void mpt_float80_decode(size_t len, const MPT_STRUCT(float80) *src, long double *dst)
{
	while (len--) {
		uint8_t *ptr = (uint8_t *) dst++;
		size_t i;
		for (i = 0; i < sizeof(src->_d); i++) ptr[i] = src->_d[i];
		++src;
	}
}
/*!
 * \ingroup mptConvert
 * \brief compact extended precision float values
 * 
 * Convert native long double to shortened format.
 * 
 * \param len number of elements
 * \param src native long double values
 * \param dst 10 byte float values
 */
extern void mpt_float80_encode(size_t len, const long double *src, MPT_STRUCT(float80) *dst)
{
	while (len--) {
		uint8_t *ptr = (uint8_t *) src++;
		size_t i;
		for (i = 0; i < sizeof(dst->_d); i++) dst->_d[i] = ptr[i];
		++dst;
	}
}
