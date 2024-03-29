/*!
 * message value parameters
 */

#include "types.h"

#include "message.h"

/*!
 * \ingroup mptMessage
 * \brief get element size
 * 
 * Calculate total element bytes
 * for transport format code.
 * 
 * \param fmt  message content format code
 * 
 * \return single element size
 */
extern size_t mpt_msgvalfmt_size(uint8_t fmt)
{
	fmt &= ~MPT_MESGVAL(ByteOrderLittle);
	if (fmt & MPT_MESGVAL(Normal)) {
		return (fmt & 0x1f) + 1;
	}
	return ((fmt & 0x1f) + 1) * MPT_MESGVAL(BigAtom);
}
/*!
 * \ingroup mptMessage
 * \brief get native type
 * 
 * Native type for transport format code.
 * Fail when byte orders diverge or
 * no local representation exists.
 * 
 * \param fmt  message value format
 * 
 * \return local type code
 */
extern int mpt_msgvalfmt_typeid(uint8_t fmt)
{
	ssize_t size;
	/* value in wrong byte order */
	if ((fmt & MPT_MESGVAL(ByteOrderLittle)) != MPT_MESGVAL(ByteOrderNative)) {
		return MPT_ERROR(BadValue);
	}
	size = mpt_msgvalfmt_size(fmt);
	
	switch (fmt & MPT_MESGVAL(Normal)) {
	  /* no representation for big numbers */
	  case 0:
		return MPT_ERROR(BadType);
	  case MPT_MESGVAL(Integer):
		fmt = mpt_type_int(size);
		break;
	  case MPT_MESGVAL(Float):
		switch (size) {
		  case sizeof(float): return 'f';
		  case sizeof(double): return 'd';
		  case sizeof(long double): return 'e';
		  default: return MPT_ERROR(BadType);
		}
	  case MPT_MESGVAL(Unsigned):
		fmt = mpt_type_uint(size);
		break;
	}
	return fmt ? fmt : MPT_ERROR(BadType);
}

/*!
 * \ingroup mptMessage
 * \brief get type message code
 * 
 * Message type code for native value type.
 * 
 * \param type  native type id
 * 
 * \return message type code
 */
extern int mpt_msgvalfmt_code(int type)
{
	switch (type) {
#ifdef _MPT_FLOAT_EXTENDED_H
	    case 'e': return MPT_message_value(Float, long double);
#endif
	    case 'd': return MPT_message_value(Float, double);
	    case 'f': return MPT_message_value(Float, float);
	    
	    case 'b': return MPT_message_value(Integer, int8_t);
	    case 'n': return MPT_message_value(Integer, int16_t);
	    case 'i': return MPT_message_value(Integer, int32_t);
	    case 'x': return MPT_message_value(Integer, int64_t);
	    
	    case 'y': return MPT_message_value(Unsigned, uint8_t);
	    case 'q': return MPT_message_value(Unsigned, uint16_t);
	    case 'u': return MPT_message_value(Unsigned, uint32_t);
	    case 't': return MPT_message_value(Unsigned, uint64_t);
	    
	    default:  return -1;
	}
}
