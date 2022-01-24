/*!
 * \file
 * byte order swap operations
 */

#if defined(__FreeBSD__) || defined(__DragonFly__)
# include <sys/endian.h>
# define bswap_64(x) bswap64(x)
# define bswap_32(x) bswap32(x)
# define bswap_16(x) bswap16(x)
#elif defined(__OpenBSD__) || defined(__Bitrig__)
# include <sys/types.h>
# define bswap_64(x) swap64(x)
# define bswap_32(x) swap32(x)
# define bswap_16(x) swap16(x)
#else
# include <byteswap.h>
#endif

#include "types.h"

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
extern void mpt_bswap_80(long len, MPT_STRUCT(float80) *val)
{
	long i;
	for (i = 0; i < len; ++i) {
		uint8_t tmp[sizeof(*val)], *ptr = (uint8_t *) val++;
		unsigned pos;
		for (pos = 0; pos < sizeof(tmp); pos++) {
			tmp[pos] = ptr[pos];
		}
		for (pos = 0; pos < sizeof(tmp); pos++) {
			ptr[pos] = tmp[sizeof(tmp) - 1 - pos];
		}
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
extern void mpt_bswap_64(long len, uint64_t *val)
{
	long i;
	
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
extern void mpt_bswap_32(long len, uint32_t *val)
{
	long i;
	
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
extern void mpt_bswap_16(long len, uint16_t *val)
{
	long i;
	
	for (i = 0; i < len; ++i) {
		val[i] = bswap_16(val[i]);
	}
}
