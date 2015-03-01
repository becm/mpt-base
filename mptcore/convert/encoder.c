#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief get encoder
 * 
 * Select encoder function for matching coding.
 * 
 * \param code	message coding
 * 
 * \return encoder function
 */
extern MPT_TYPE(DataEncoder) mpt_message_encoder(int code)
{
	switch (code) {
	  /* no encoding */
	  case 0:
	    return 0;
	  /* zero-terminated data */
	  case MPT_ENUM(EncodingCommand):
	    return mpt_encode_string;
	  /* normal COBS output */
	  case MPT_ENUM(EncodingCobs):
	    return mpt_encode_cobs;
	  /* COBS with tail inline */
	  case MPT_ENUM(EncodingCobsInline):
	    return mpt_encode_cobs_r;
	  /* compressed COBS output */
	  case MPT_ENUM(EncodingCobs) | MPT_ENUM(EncodingCompress):
	    return mpt_encode_cobs_zpe;
	  /* compressed COBS with tail inline */
	  case MPT_ENUM(EncodingCobsInline) | MPT_ENUM(EncodingCompress):
	    return mpt_encode_cobs_zpe_r;
	  /* invalid encoding */
	  default: return 0;
	}
}

