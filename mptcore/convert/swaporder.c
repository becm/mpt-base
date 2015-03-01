/*!
 * \file
 * byte order swap operations
 */

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief 80 bit swap
 * 
 * Swap extended float byte order
 * 
 * \param len number of elements
 * \param val start address of data
 */
extern void mpt_swaporder_80(size_t len, MPT_STRUCT(extflt) *val)
{
	while (len--) {
		size_t i;
		uint8_t tmp[sizeof(*val)], *ptr = (val++)->_d;
		for (i = 0; i < sizeof(tmp); i++) tmp[i] = ptr[i];
		for (i = 0; i < sizeof(tmp); i++) ptr[i] = tmp[sizeof(tmp)-1-i];
		++val;
	}
}
/*!
 * \ingroup mptConvert
 * \brief 64 bit swap
 * 
 * Swap 64bit data byte order
 * 
 * \param len number of elements
 * \param val start address of data
 */
extern void mpt_swaporder_64(size_t len, uint64_t *val)
{
	while (len--) {
		size_t i;
		uint8_t tmp[sizeof(*val)], *ptr = (uint8_t *) val++;
		for (i = 0; i < sizeof(tmp); i++) tmp[i] = ptr[i];
		for (i = 0; i < sizeof(tmp); i++) ptr[i] = tmp[sizeof(tmp)-1-i];
	}
}
/*!
 * \ingroup mptConvert
 * \brief 32 bit swap
 * 
 * Swap 32bit data byte order
 * 
 * \param len number of elements
 * \param val start address of data
 */
extern void mpt_swaporder_32(size_t len, uint32_t *val)
{
	while (len--) {
		size_t i;
		uint8_t tmp[sizeof(*val)], *ptr = (uint8_t *) val++;
		for (i = 0; i < sizeof(tmp); i++) tmp[i] = ptr[i];
		for (i = 0; i < sizeof(tmp); i++) ptr[i] = tmp[sizeof(tmp)-1-i];
	}
}
/*!
 * \ingroup mptConvert
 * \brief 16 bit swap
 * 
 * Swap 16bit data byte order
 * 
 * \param len number of elements
 * \param val start address of data
 */
extern void mpt_swaporder_16(size_t len, uint16_t *val)
{
	while (len--) {
		uint8_t tmp, *ptr = (uint8_t *) val++;
		tmp    = ptr[0];
		ptr[0] = ptr[1];
		ptr[1] = tmp;
	}
}
