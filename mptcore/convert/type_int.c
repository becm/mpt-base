/*!
 * integer types for sizes
 */

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief id for integer
 * 
 * Integer type ID for specified byte size.
 * 
 * \return type ID for 'signed' integer
 */
extern char mpt_type_int(size_t len)
{
	switch (len) {
	  case sizeof(int8_t):  return 'b';
	  case sizeof(int16_t): return 'n';
	  case sizeof(int32_t): return 'i';
	  case sizeof(int64_t): return 'x';
	  default: return 0;
	}
}
/*!
 * \ingroup mptConvert
 * \brief id for unsigned
 * 
 * Unsigned integer type ID for specified byte size.
 * 
 * \return type ID for 'unsigned' integer
 */
extern char mpt_type_uint(size_t len)
{
	switch (len) {
	  case sizeof(uint8_t):  return 'y';
	  case sizeof(uint16_t): return 'q';
	  case sizeof(uint32_t): return 'u';
	  case sizeof(uint64_t): return 't';
	  default: return 0;
	}
}
