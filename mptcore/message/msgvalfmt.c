/*!
 * message value parameters
 */

#include "message.h"

#include "convert.h"

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
	if (fmt & MPT_MSGVAL(Normal)) {
		return (fmt & 0x1f) + 1;
	}
	return ((fmt & 0x1f) + 1) * MPT_MSGVAL(BigAtom);
}
/*!
 * \ingroup mptMessage
 * \brief native type
 * 
 * get native type for transport
 * 
 * \param fmt  message value format
 * 
 * \return local type representation
 */
extern int mpt_msgvalfmt_type(uint8_t fmt)
{
	ssize_t size;
	/* value in wrong byte order */
	if ((fmt & MPT_ENUM(ByteOrderLittle)) != MPT_ENUM(ByteOrderNative)) {
		return MPT_ERROR(BadValue);
	}
	size = mpt_msgvalfmt_size(fmt);
	
	switch (fmt & MPT_MSGVAL(Normal)) {
	  /* no representation for big numbers */
	  case 0:
		return MPT_ERROR(BadType);
	  case MPT_MSGVAL(Integer):
		fmt = mpt_type_int(size);
		break;
	  case MPT_MSGVAL(Float):
		switch (size) {
		  case  4: return 'f';
		  case  8: return 'd';
		  case 10: return MPT_ENUM(TypeFloat80);
		  case sizeof(long double): return 'e';
		  default: return MPT_ERROR(BadType);
		}
	  case MPT_MSGVAL(Unsigned):
		fmt = mpt_type_uint(size);
		break;
	}
	return fmt ? fmt : MPT_ERROR(BadType);
}
