/*!
 * MPT core library
 *  file/network/data interface and string conversion
 */

#ifndef _MPT_CONVERT_H
#define _MPT_CONVERT_H  @INTERFACE_VERSION@

#include "core.h"

#if _XOPEN_SOURCE >= 600 || __STDC_VERSION__ >= 199901L || _POSIX_C_SOURCE >= 200112L
# define _MPT_FLOAT_EXTENDED_H
#endif

__MPT_NAMESPACE_BEGIN

MPT_INTERFACE(metatype);

enum MPT_ENUM(EncodingType) {
	MPT_ENUM(EncodingCommand)      = 0x1,   /* terminate by zero byte */
	MPT_ENUM(EncodingCobs)         = 0x2,   /* use cobs encoding */
	MPT_ENUM(EncodingCobsInline)   = 0x3,   /* cobs with tail inline */
	MPT_ENUM(EncodingCompress)     = 0x4    /* compress data */
};
enum MPT_ENUM(NewlineTypes) {
	MPT_ENUM(NewlineMac)  = 0x1,       /* MacOS line separation */
	MPT_ENUM(NewlineUnix) = 0x2,       /* UNIX line separation */
	MPT_ENUM(NewlineNet)  = 0x3        /* network/Windows line separation */
};

MPT_STRUCT(float80)
{
#ifdef __cplusplus
public:
	inline float80() {}
	inline float80(long double v) { *this = v; }
	
	float80 &operator =(long double);
	long double value() const;
protected:
#endif
	uint8_t _d[10];
};
#ifdef __cplusplus
template<> inline __MPT_CONST_EXPR int typeIdentifier<float80>()  { return TypeFloat80; }
float swapOrder(float);
double swapOrder(double);
float80 swapOrder(float80);
#endif

/* value output format */
#ifdef __cplusplus
MPT_STRUCT(valfmt)
{
public:
	inline valfmt() : fmt(6), wdt(0)
	{ }
	inline int width() const
	{ return wdt; }
	inline int flags() const
	{ return fmt & 0xff00; }
	inline int decimals() const
	{ return fmt & 0x7f; }
# define MPT_VALFMT(x)  x
#else
# define MPT_VALFMT(x)  MPT_ENUM(Format##x)
#endif
enum MPT_VALFMT(Flags) {
	MPT_VALFMT(IntHex)      = 0x0100, /* print hexadecimal */
	MPT_VALFMT(IntOctal)    = 0x0200, /* print octal integer */
	MPT_VALFMT(FltHex)      = 0x0400, /* print hexadecimal */
	MPT_VALFMT(NumberHex)   = MPT_VALFMT(IntHex) | MPT_VALFMT(FltHex),
	MPT_VALFMT(Scientific)  = 0x0800, /* scientific float notation */
	
	MPT_VALFMT(Sign)        = 0x1000, /* print sign */
	MPT_VALFMT(Left)        = 0x2000  /* print left bounded */
};
#ifdef __cplusplus
protected:
#else
MPT_STRUCT(valfmt)
{
# define MPT_VALFMT_INIT  { 0, 0 }
# define MPT_VALFMT_DECMAX  0x7f
#endif
	uint16_t fmt;  /* format flags and number of decimals */
	uint8_t  wdt;  /* field width */
};

__MPT_EXTDECL_BEGIN

/* extended double conversions */
extern void mpt_float80_decode(size_t , const MPT_STRUCT(float80) *, long double *);
extern void mpt_float80_encode(size_t , const long double *, MPT_STRUCT(float80) *);
/* byte order conversion */
extern void mpt_bswap_80(size_t , MPT_STRUCT(float80) *);
extern void mpt_bswap_64(size_t , uint64_t *);
extern void mpt_bswap_32(size_t , uint32_t *);
extern void mpt_bswap_16(size_t , uint16_t *);

/* get data from pointer and description */
extern int mpt_data_convert(const void **, int , void *, int );
/* get string data */
extern const char *mpt_data_tostring(const void **, int , size_t *);

/* get number type from string */
extern int mpt_convert_number(const char *, int , void *);

/* get keyword/type from text */
extern const char *mpt_convert_key(const char **, const char *, size_t *);
extern int mpt_convert_string(const char *, int , void *);

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
extern int mpt_cint (int  *, const char *, int , const int  [2]);
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
extern ssize_t mpt_encode_cobs(MPT_STRUCT(encode_state) *, const struct iovec *, const struct iovec *);
extern ssize_t mpt_encode_cobs_r(MPT_STRUCT(encode_state) *, const struct iovec *, const struct iovec *);
extern ssize_t mpt_decode_cobs(MPT_STRUCT(decode_state) *, const struct iovec *, size_t);
extern ssize_t mpt_decode_cobs_r(MPT_STRUCT(decode_state) *, const struct iovec *, size_t);
extern int mpt_cobs_dec(const void *, size_t *, void *, size_t *);
/* COBS/ZPE(+R) operations */
extern ssize_t mpt_encode_cobs_zpe(MPT_STRUCT(encode_state) *, const struct iovec *, const struct iovec *);
extern ssize_t mpt_encode_cobs_zpe_r(MPT_STRUCT(encode_state) *, const struct iovec *, const struct iovec *);
extern ssize_t mpt_decode_cobs_zpe(MPT_STRUCT(decode_state) *, const struct iovec *, size_t);
extern ssize_t mpt_decode_cobs_zpe_r(MPT_STRUCT(decode_state) *, const struct iovec *, size_t);
extern int mpt_cobs_dec_zpe(const void *, size_t *, void *, size_t *);

/* filter non-zero inline data */
extern ssize_t mpt_encode_string(MPT_STRUCT(encode_state) *, const struct iovec *, const struct iovec *);
/* add command header to zero-terminated text */
extern ssize_t mpt_decode_command(MPT_STRUCT(decode_state) *, const struct iovec *, size_t);


/* print all properties */
extern int mpt_generic_print(int (*)(void *, MPT_STRUCT(property) *), void *, MPT_TYPE(PropertyHandler) , void *, int __MPT_DEFPAR(0));

/* convert structured data to string */
extern int mpt_number_print(char *, size_t , MPT_STRUCT(valfmt) , int , const void *);
/* output data */
extern int mpt_tostring(const MPT_STRUCT(value) *, ssize_t (*)(void *, const char *, size_t), void *);

/* parse/create terminal output format */
extern int mpt_valfmt_get(MPT_STRUCT(valfmt) *, const char *);
#ifdef _MPT_ARRAY_H
extern int mpt_valfmt_parse(_MPT_ARRAY_TYPE(valfmt) *, const char *);
extern int mpt_valfmt_set(_MPT_ARRAY_TYPE(valfmt) *, MPT_INTERFACE(metatype) *);
#endif

/* type identifier for '(unsigned) long' data type */
extern char mpt_type_int(size_t);
extern char mpt_type_uint(size_t);

/* line end separator */
extern const char *mpt_newline_string(int);
extern int mpt_newline_native(void);

__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#ifdef __cplusplus
inline std::ostream &operator<<(std::ostream &o, const mpt::float80 &f)
{ return o << static_cast<double>(f.value()); }
#endif

#endif /* _MPT_CONVERT_H */

