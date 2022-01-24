/*!
 * \file
 * 80bit extended float handling
 */

#include "types.h"

#include <ieee754.h>

/**
 * Do NOT merge prefix and mantissa structs,
 * data alignment WILL break on BigEndian!
 * 
 * Order of prefix and mantissa is defined in struct.
 * 
 * Mirror word order from IEEE for consistent byte order
 * on conversion/representation.
 */

/* definition for sign and exponent */
struct _mpt_ieee_float80_prefix
{
#if __BYTE_ORDER == __BIG_ENDIAN
	unsigned int negative:1;
	unsigned int exponent:15;
#endif
#if __BYTE_ORDER == __LITTLE_ENDIAN
	unsigned int exponent:15;
	unsigned int negative:1;
#endif
};
/* definition for mantissa components */
struct _mpt_ieee_float80_mantissa
{
#if __BYTE_ORDER == __BIG_ENDIAN || __FLOAT_WORD_ORDER == __BIG_ENDIAN
	unsigned int mantissa0:32;
	unsigned int mantissa1:32;
#endif
#if __BYTE_ORDER == __LITTLE_ENDIAN
	unsigned int mantissa1:32;
	unsigned int mantissa0:32;
#endif
};

/*!
 * \ingroup mptTypes
 * \brief get extended precision float values
 * 
 * Convert shortened to native long double format.
 * 
 * \param len number of elements
 * \param src 10 byte float values
 * \param dst native long double values
 */
extern void mpt_float80_decode(long len, const MPT_STRUCT(float80) *from, long double *to)
{
	long pos;
	for (pos = 0; pos < len; pos++) {
		const struct _mpt_ieee_float80_mantissa *m = (const void *) &from[pos]._mantissa;
		const struct _mpt_ieee_float80_prefix *p = (const void *) &from[pos]._prefix;
		union ieee854_long_double *dst = (void *) (to + pos);
		
		dst->ieee.mantissa0 = m->mantissa0;
		dst->ieee.mantissa1 = m->mantissa1;
		dst->ieee.exponent  = p->exponent;
		dst->ieee.negative  = p->negative;
		dst->ieee.empty = 0;
	}
}
/*!
 * \ingroup mptTypes
 * \brief compact extended precision float values
 * 
 * Convert native long double to shortened format.
 * 
 * \param len number of elements
 * \param src native long double values
 * \param dst 10 byte float values
 */
extern void mpt_float80_encode(long len, const long double *from, MPT_STRUCT(float80) *to)
{
	long pos;
	for (pos = 0; pos < len; pos++) {
		const union ieee854_long_double *src = (const void *) from++;
		struct _mpt_ieee_float80_mantissa *m = (void *) &to[pos]._mantissa;
		struct _mpt_ieee_float80_prefix *p = (void *) &to[pos]._prefix;
		
		m->mantissa0 = src->ieee.mantissa0;
		m->mantissa1 = src->ieee.mantissa1;
		p->exponent  = src->ieee.exponent;
		p->negative  = src->ieee.negative;
	}
}
/*!
 * \ingroup mptTypes
 * \brief compare compact extended precision
 * 
 * Compare compact 80bit representation of extended float.
 * Semantics similar to strcmp and spaceship operator;
 *   prefix diff returns ±1,
 *   mantissa diff returns ±2.
 * 
 * \param val value to classify
 * \param ref reference value
 * 
 * \return relation of values
 */
extern int mpt_float80_compare(const MPT_STRUCT(float80) *val, const MPT_STRUCT(float80) *ref)
{
	const struct _mpt_ieee_float80_mantissa *val_m, *ref_m;
	/* sign and exponent */
	if (val->_prefix < ref->_prefix) {
		return -1;
	}
	if (val->_prefix > ref->_prefix) {
		return 1;
	}
	val_m = (const void *) val->_mantissa;
	ref_m = (const void *) ref->_mantissa;
	/* mantissa start */
	if (val_m->mantissa0 < ref_m->mantissa0) {
		return -2;
	}
	if (val_m->mantissa0 > ref_m->mantissa0) {
		return 2;
	}
	/* mantissa continuation */
	if (val_m->mantissa1 < ref_m->mantissa1) {
		return -2;
	}
	if (val_m->mantissa1 > ref_m->mantissa1) {
		return 2;
	}
	return 0;
}
	


