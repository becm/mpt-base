/*!
 * \file
 * byte order swap operations
 */

#if defined(__FreeBSD__)
# include <sys/endian.h>
# define bswap_64(x) bswap64(x)
# define bswap_32(x) bswap32(x)
# define bswap_16(x) bswap16(x)
#else
# include <byteswap.h>
#endif

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
extern void mpt_bswap_80(size_t len, MPT_STRUCT(float80) *val)
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
extern void mpt_bswap_64(size_t len, uint64_t *val)
{
	size_t i;
	
	for (i = 0; i < len; ++i) {
		val[i] = bswap_64(val[i]);
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
extern void mpt_bswap_32(size_t len, uint32_t *val)
{
	size_t i;
	
	for (i = 0; i < len; ++i) {
		val[i] = bswap_32(val[i]);
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
extern void mpt_bswap_16(size_t len, uint16_t *val)
{
	size_t i;
	
	for (i = 0; i < len; ++i) {
		val[i] = bswap_32(val[i]);
	}
}
