/*!
 * MPT core library
 *  file/network/data interface and string conversion
 */

#ifndef _MPT_CONVERT_H
#define _MPT_CONVERT_H  201502

#include "core.h"

__MPT_NAMESPACE_BEGIN

enum MPT_ENUM(EncodingType) {
	MPT_ENUM(EncodingCommand)      = 0x1,   /* terminate by zero byte */
	MPT_ENUM(EncodingCobs)         = 0x2,   /* use cobs encoding */
	MPT_ENUM(EncodingCobsInline)   = 0x3,   /* cobs with tail inline */
	MPT_ENUM(EncodingCompress)     = 0x4,   /* compress data */
};

MPT_STRUCT(float80)
{
#ifdef __cplusplus
public:
	inline float80()
	{ for (size_t i = 0; i < sizeof(_d); i++) _d[i] = 0; }
	inline float80(const float80 &v)
	{ for (size_t i = 0; i < sizeof(_d); i++) _d[i] = v._d[i]; }
	inline float80(const long double &v)
	{ *this = v; }
	
	float80 &operator =(const long double &);
	long double value(void) const;
	float80 &swapOrder(void);
private:
#endif
	uint8_t _d[10];
};

__MPT_EXTDECL_BEGIN

/* get basic types from string */
extern int mpt_convert(const char *, int , void *);

/* get data from pointer and description */
extern int mpt_data_convert(const void **, int , void *, int );

/* extended double conversions */
extern void mpt_float80_decode(size_t , const MPT_STRUCT(float80) *, long double *);
extern void mpt_float80_encode(size_t , const long double *, MPT_STRUCT(float80) *);
/* byte order conversion */
extern void mpt_bswap_80(size_t , MPT_STRUCT(float80) *);
extern void mpt_bswap_64(size_t , uint64_t *);
extern void mpt_bswap_32(size_t , uint32_t *);
extern void mpt_bswap_16(size_t , uint16_t *);

/* get keyword/type from text */
extern const char *mpt_convert_key(const char **, const char *, size_t *);
extern int mpt_convert_string(const char **, int , void *);

/* string conversions */
extern int _mpt_convert_int(void *, size_t , const char *, int);
extern int _mpt_convert_uint(void *, size_t , const char *, int);

extern int mpt_cldouble(long double *, const char *, const long double [2]);
extern int mpt_cdouble(double *, const char *, const double [2]);
extern int mpt_cfloat(float *, const char *, const float [2]);

extern int mpt_cint64(int64_t *, const char *, int , const int64_t [2]);
extern int mpt_cint32(int32_t *, const char *, int , const int32_t [2]);
extern int mpt_cint16(int16_t *, const char *, int , const int16_t [2]);
extern int mpt_cint8 (int8_t  *, const char *, int , const int8_t  [2]);

extern int mpt_cchar(char *, const char *, int , const char [2]);
extern int mpt_cint(int  *, const char *, int , const int  [2]);
extern int mpt_clong(long *, const char *, int , const long [2]);

extern int mpt_cuint64(uint64_t *, const char *, int , const uint64_t [2]);
extern int mpt_cuint32(uint32_t *, const char *, int , const uint32_t [2]);
extern int mpt_cuint16(uint16_t *, const char *, int , const uint16_t [2]);
extern int mpt_cuint8 (uint8_t  *, const char *, int , const uint8_t  [2]);

extern int mpt_cuchar(unsigned char *, const char *, int , const unsigned char [2]);
extern int mpt_cuint (unsigned int  *, const char *, int , const unsigned int  [2]);
extern int mpt_culong(unsigned long *, const char *, int , const unsigned long [2]);

/* decode (multibyte) utf8 character */
extern int mpt_cutf8(const char **, size_t);

/* calculate smdb-hash for data */
extern uintptr_t mpt_hash_smdb(const void *, size_t);
/* calculate djb2-hash for data */
extern uintptr_t mpt_hash_djb2(const void *, size_t);
/* set hash type */
extern int _mpt_hash_set(const char *);

/* en/decoder selection */
extern MPT_TYPE(DataEncoder) mpt_message_encoder(int);
extern MPT_TYPE(DataDecoder) mpt_message_decoder(int);

/* type of encoding */
extern int mpt_encoding_value(const char *, int __MPT_DEFPAR(-1));
extern const char *mpt_encoding_type(int);

/* standard COBS(/R) operations */
extern ssize_t mpt_encode_cobs(MPT_STRUCT(codestate) *, const struct iovec *, const struct iovec *);
extern ssize_t mpt_encode_cobs_r(MPT_STRUCT(codestate) *, const struct iovec *, const struct iovec *);
extern ssize_t mpt_decode_cobs(MPT_STRUCT(codestate) *, const struct iovec *, size_t);
extern ssize_t mpt_decode_cobs_r(MPT_STRUCT(codestate) *, const struct iovec *, size_t);
extern int mpt_cobs_dec(const void *, size_t *, void *, size_t *);
/* COBS/ZPE(+R) operations */
extern ssize_t mpt_encode_cobs_zpe(MPT_STRUCT(codestate) *, const struct iovec *, const struct iovec *);
extern ssize_t mpt_encode_cobs_zpe_r(MPT_STRUCT(codestate) *, const struct iovec *, const struct iovec *);
extern ssize_t mpt_decode_cobs_zpe(MPT_STRUCT(codestate) *, const struct iovec *, size_t);
extern ssize_t mpt_decode_cobs_zpe_r(MPT_STRUCT(codestate) *, const struct iovec *, size_t);
extern int mpt_cobs_dec_zpe(const void *, size_t *, void *, size_t *);

/* filter non-zero inline data */
extern ssize_t mpt_encode_string(MPT_STRUCT(codestate) *, const struct iovec *, const struct iovec *);
/* add command header to zero-terminated text */
extern ssize_t mpt_decode_command(MPT_STRUCT(codestate) *, const struct iovec *, size_t);


/* loop trough properties */
extern int mpt_generic_foreach(MPT_TYPE(PropertyHandler) , void *, MPT_TYPE(PropertyHandler) , void *, int __MPT_DEFPAR(0));
extern int mpt_generic_print(MPT_TYPE(PropertyHandler) , void *, MPT_TYPE(PropertyHandler) , void *, int __MPT_DEFPAR(0));

/* convert structured data to string */
extern int mpt_data_print(char *, size_t , int , const void **);
/* output data */
extern int mpt_tostring(const void *, const char *, int (*)(void *, const char *, size_t ), void *);


/* get type position from data description */
extern int mpt_position(const char *, int);
/* get position offset from data description */
extern int mpt_offset(const char *, int);

/* get/add registered (primitive) types */
extern ssize_t mpt_valsize(int);
extern int mpt_valtype_add(size_t);


__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_CONVERT_H */

