/*!
 * MPT core library
 *  file/network/data interface and string conversion
 */

#ifndef _MPT_CONVERT_H
#define _MPT_CONVERT_H  @INTERFACE_VERSION@

#ifdef __cplusplus
# include "types.h"
#else
# include "core.h"
#endif

struct iovec;

#if _XOPEN_SOURCE >= 600 || __STDC_VERSION__ >= 199901L || _POSIX_C_SOURCE >= 200112L
# define _MPT_FLOAT_EXTENDED_H
#endif

__MPT_NAMESPACE_BEGIN

MPT_INTERFACE(metatype);
MPT_INTERFACE(object);

MPT_STRUCT(array);
MPT_STRUCT(value);
MPT_STRUCT(float80);

enum MPT_ENUM(EncodingType) {
	MPT_ENUM(EncodingCommand)      = 0x1,   /* terminate by zero byte */
	MPT_ENUM(EncodingCobs)         = 0x2,   /* use cobs encoding */
	MPT_ENUM(EncodingCobsInline)   = 0x3,   /* cobs with tail inline */
	MPT_ENUM(EncodingCompress)     = 0x4    /* compress data */
};
enum MPT_ENUM(NewlineTypes) {
	MPT_ENUM(NewlineMac)  = 0x1,  /* MacOS line separation */
	MPT_ENUM(NewlineUnix) = 0x2,  /* UNIX line separation */
	MPT_ENUM(NewlineNet)  = 0x3   /* network/Windows line separation */
};

/* value output format */
#ifdef __cplusplus
MPT_STRUCT(value_format)
{
public:
	inline value_format() : flags(0), width(0), dec(6)
	{ }
# define MPT_VALFMT(x)  x
#else
# define MPT_VALFMT(x)  MPT_ENUM(Format##x)
#endif
enum MPT_VALFMT(Flags) {
	MPT_VALFMT(IntHex)      = 0x1,    /* print hexadecimal */
	MPT_VALFMT(IntOctal)    = 0x2,    /* print octal integer */
	MPT_VALFMT(IntFlags)    = 0xf,    /* integer format mask */
	MPT_VALFMT(FltHex)      = 0x10,   /* print hexadecimal */
	MPT_VALFMT(Scientific)  = 0x20,   /* scientific float notation */
	MPT_VALFMT(FltFlags)    = 0xf0,   /* float format mask */
	MPT_VALFMT(NumberHex)   = MPT_VALFMT(IntHex) | MPT_VALFMT(FltHex),
	
	MPT_VALFMT(Sign)        = 0x100,  /* print sign */
	MPT_VALFMT(Left)        = 0x200   /* print left bounded */
};

#ifndef __cplusplus
MPT_STRUCT(value_format)
{
# define MPT_VALFMT_INIT  { 0, 0, 0 }
#endif
	uint16_t flags;  /* format flags */ 
	uint8_t  width;  /* field width */
	uint8_t  dec;    /* number of decimals */
};

MPT_STRUCT(strdest)
{
	uint8_t change,  /* positions which were changed */
	        val[7];  /* values before/after reading */
};

typedef int (*MPT_TYPE(data_converter))(const void *, int , void *);

#ifdef __cplusplus
template<> inline __MPT_CONST_TYPE int type_properties<value_format>::id(bool) {
	return TypeValFmt;
}
template <> inline const struct type_traits *type_properties<value_format>::traits() {
	return type_traits::get(id(true));
}

float swapOrder(float);
double swapOrder(double);
float80 swapOrder(float80);
#endif

__MPT_EXTDECL_BEGIN

/* byte order conversion */
extern void mpt_bswap_80(long , MPT_STRUCT(float80) *);
extern void mpt_bswap_64(long , uint64_t *);
extern void mpt_bswap_32(long , uint32_t *);
extern void mpt_bswap_16(long , uint16_t *);

/* get data from pointer and description */
extern int mpt_value_convert(const MPT_STRUCT(value) *, int , void *);
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

/* integer converters */
extern int mpt_data_convert_int8 (const int8_t  *, int , void *);
extern int mpt_data_convert_int16(const int16_t *, int , void *);
extern int mpt_data_convert_int32(const int32_t *, int , void *);
extern int mpt_data_convert_int64(const int64_t *, int , void *);
/* unsigned integer converters */
extern int mpt_data_convert_uint8 (const uint8_t  *, int , void *);
extern int mpt_data_convert_uint16(const uint16_t *, int , void *);
extern int mpt_data_convert_uint32(const uint32_t *, int , void *);
extern int mpt_data_convert_uint64(const uint64_t *, int , void *);
/* floating point converters */
extern int mpt_data_convert_float32(const float  *, int , void *);
extern int mpt_data_convert_float64(const double *, int , void *);
#ifdef _MPT_FLOAT_EXTENDED_H
extern int mpt_data_convert_exflt(const long double *, int , void *);
#endif
extern int mpt_data_convert_array(const MPT_STRUCT(array) *, int , void *);

/* data converter resolution */
MPT_TYPE(data_converter) mpt_data_converter(int);


/* en/decoder selection */
extern MPT_TYPE(data_encoder) mpt_message_encoder(int);
extern MPT_TYPE(data_decoder) mpt_message_decoder(int);

/* type of encoding */
extern int mpt_encoding_value(const char *, int __MPT_DEFPAR(-1));
extern const char *mpt_encoding_type(int);

/* standard COBS(/R) operations */
extern ssize_t mpt_encode_cobs(MPT_STRUCT(encode_state) *, const struct iovec *, const struct iovec *);
extern ssize_t mpt_encode_cobs_r(MPT_STRUCT(encode_state) *, const struct iovec *, const struct iovec *);
extern int mpt_decode_cobs(MPT_STRUCT(decode_state) *, const struct iovec *, size_t);
extern int mpt_decode_cobs_r(MPT_STRUCT(decode_state) *, const struct iovec *, size_t);
extern int mpt_cobs_dec(const void *, size_t *, void *, size_t *);
/* COBS/ZPE(+R) operations */
extern ssize_t mpt_encode_cobs_zpe(MPT_STRUCT(encode_state) *, const struct iovec *, const struct iovec *);
extern ssize_t mpt_encode_cobs_zpe_r(MPT_STRUCT(encode_state) *, const struct iovec *, const struct iovec *);
extern int mpt_decode_cobs_zpe(MPT_STRUCT(decode_state) *, const struct iovec *, size_t);
extern int mpt_decode_cobs_zpe_r(MPT_STRUCT(decode_state) *, const struct iovec *, size_t);
extern int mpt_cobs_dec_zpe(const void *, size_t *, void *, size_t *);

/* filter non-zero inline data */
extern ssize_t mpt_encode_string(MPT_STRUCT(encode_state) *, const struct iovec *, const struct iovec *);
/* add command header to zero-terminated text */
extern int mpt_decode_command(MPT_STRUCT(decode_state) *, const struct iovec *, size_t);


/* convert structured data to string */
extern int mpt_number_tostring(const MPT_STRUCT(value) *, MPT_STRUCT(value_format) , char *, size_t);
/* output data */
extern int mpt_print_value(const MPT_STRUCT(value) *, ssize_t (*)(void *, const char *, size_t), void *);
extern int mpt_print_object(const MPT_INTERFACE(object) *, ssize_t (*)(void *, const char *, size_t), void *);
extern int mpt_print_convertable(MPT_INTERFACE(convertable) *, ssize_t (*)(void *, const char *, size_t), void *);

/* parse/create terminal output format */
extern int mpt_valfmt_get(MPT_STRUCT(value_format) *, const char *);
#ifdef _MPT_ARRAY_H
extern int mpt_valfmt_parse(_MPT_ARRAY_TYPE(value_format) *, const char *);
extern int mpt_valfmt_set(_MPT_ARRAY_TYPE(value_format) *, MPT_INTERFACE(convertable) *);
extern int mpt_valfmt_add(_MPT_ARRAY_TYPE(value_format) *, MPT_STRUCT(value_format));
#endif

/* parse character separated values */
extern int mpt_string_dest(MPT_STRUCT(strdest) *, int , const char *);

/* line end separator */
extern const char *mpt_newline_string(int);
extern int mpt_newline_native(void);

__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_CONVERT_H */

