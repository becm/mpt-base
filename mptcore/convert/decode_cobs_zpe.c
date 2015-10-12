

#define MPT_COBS_MAXLEN 0xdf  /* = 223 */

#define MPT_cobs_data_len(c)   (((c) <= MPT_COBS_MAXLEN) ? (c) - 1 : (c) - 0xe0)
#define MPT_cobs_zero_len(c,n) (((c) >= 0xe0) ? 2 : ((((c) < MPT_COBS_MAXLEN) && (n)) ? 1 : 0))
#define MPT_cobs_max_dec(c)    ((c) * 2)

#define MPT_cobs_dec_regular mpt_decode_cobs_zpe

#include "decode_cobs.c"


/*!
 * \ingroup mptConvert
 * \brief decode COBS/ZPE data
 * 
 * Decode data in COBS format in vector array.
 * 
 * New message data begins at \info.done ,
 * encoded data continues \info.scratch later
 * 
 * Pass \sourcelen = 0 to get current message length
 * on single data element.
 * No data change is performed in this case.
 * 
 * Pass \source = 0 to get maximum required message size
 * for remaining data length.
 * No data change is performed in this case.
 * 
 * Pass \sourcelen = 0 and \source = 0 to reset
 * \codestate data.
 * 
 * \param info      start position of encoded data
 * \param source    encoded data
 * \param sourcelen length of source array
 * 
 * \return size of message
 */
extern ssize_t mpt_decode_cobs_zpe(MPT_STRUCT(codestate) *info, const struct iovec *source, size_t sourcelen)
{
	return _decode(info, source, sourcelen);
}
/*!
 * \ingroup mptConvert
 * \brief decode COBS/ZPE+R data
 * 
 * Decode data in COBS format in vector array.
 * 
 * New message data begins at \info.done ,
 * encoded data continues \info.scratch later
 * 
 * Pass \sourcelen = 0 to get current message length
 * on single data element.
 * No data change is performed in this case.
 * 
 * Pass \source = 0 to get maximum required message size
 * for remaining data length.
 * No data change is performed in this case.
 * 
 * Pass \sourcelen = 0 and \source = 0 to reset
 * \codestate data.
 * 
 * \param info      start position of encoded data
 * \param source    encoded data
 * \param sourcelen length of source array
 * 
 * \return size of message
 */
extern ssize_t mpt_decode_cobs_zpe_r(MPT_STRUCT(codestate) *info, const struct iovec *source, size_t sourcelen)
{
	return _decode_r(info, source, sourcelen);
}
