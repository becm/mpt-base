#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief get decoder
 * 
 * Select decoder function for matching coding.
 * 
 * \param code  message coding
 * 
 * \return decoder function
 */
extern MPT_TYPE(data_decoder) mpt_message_decoder(int code)
{
	switch (code & 0x7f) {
	  /* no encoding */
	  case 0:
	    return 0;
	  /* text message format */
	  case MPT_ENUM(EncodingCommand):
	    return mpt_decode_command;
	  /* normal COBS output */
	  case MPT_ENUM(EncodingCobs):
	    return mpt_decode_cobs;
	  /* COBS with tail inline */
	  case MPT_ENUM(EncodingCobsInline):
	    return mpt_decode_cobs_r;
	  /* compressed COBS output */
	  case MPT_ENUM(EncodingCobs) | MPT_ENUM(EncodingCompress):
	    return mpt_decode_cobs_zpe;
	  /* compressed COBS with tail inline */
	  case MPT_ENUM(EncodingCobsInline) | MPT_ENUM(EncodingCompress):
	    return mpt_decode_cobs_zpe_r;
	  /* invalid encoding */
	  default: return 0;
	}
}

