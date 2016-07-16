/*!
 * message value parameters
 */

#include "message.h"

/*!
 * \ingroup mptMessage
 * \brief size of format elements
 * 
 * Calculate total element bytes
 * 
 * \param fmt  message value format
 * 
 * \return total element size
 */
extern size_t mpt_msgvalfmt_size(uint8_t fmt)
{
	fmt &= ~MPT_ENUM(ByteOrderLittle);
	if (fmt >= MPT_ENUM(TypeScalBase)) {
		ssize_t nat = mpt_valsize(fmt);
		return nat < 0 ? 0 : nat;
	}
	return (fmt & MPT_ENUM(ValuesBig)) ? (0x10 << (fmt & 0xf)) : (fmt & 0xf);
}
/*!
 * \ingroup mptMessage
 * \brief native type
 * 
 * get native type for transport
 * 
 * \param fmt  message value format
 * 
 * \return total element size
 */
extern int mpt_msgvalfmt_type(uint8_t fmt)
{
	ssize_t size;
	
	if (!(size = mpt_msgvalfmt_size(fmt))) {
		return MPT_ERROR(BadArgument);
	}
	/* float types */
	if (fmt & MPT_ENUM(ValuesFloat)) {
		/* combination for native types */
		if (fmt & MPT_ENUM(ValuesUnsigned)) {
			return fmt & ~0x7f;
		}
		switch (size) {
		  case  4: return 'f';
		  case  8: return 'd';
		  case 10: return MPT_ENUM(TypeFloat80);
		  case sizeof(long double): return 'e';
		  default: return MPT_ERROR(BadType);
		}
	}
	/* unsigned integers */
	if (fmt & MPT_ENUM(ValuesUnsigned)) {
		switch (size) {
		  case sizeof(int8_t):  return 'y';
		  case sizeof(int16_t): return 'q';
		  case sizeof(int32_t): return 'u';
		  case sizeof(int64_t): return 't';
		  default: return MPT_ERROR(BadType);
		}
	}
	/* signed integers */
	else {
		switch (size) {
		  case sizeof(int8_t):  return 'b';
		  case sizeof(int16_t): return 'n';
		  case sizeof(int32_t): return 'i';
		  case sizeof(int64_t): return 'x';
		  default: return MPT_ERROR(BadType);
		}
	}
}
