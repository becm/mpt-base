/*!
 * encode/decode output format of floating point value.
 * 
 * format encoding (16bit):
 *    S: sign bit     (mask)
 *    w: field width  (7bit,(1+)0..127)
 *    d: decimals     (5bit,0..31)
 *    f: format       (3bit,(fegaFEGA))
 *    Swwwwwww dddddfff
 */

#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <ctype.h>

#include "message.h"

extern int mpt_outfmt_get(int fmt, int *width, int *dec)
{
	if (fmt < 0) return 0;
	if (width) *width = ((fmt & 0x7f00) / 0x100);
	if (dec)   *dec = (fmt & 0xf8) / 0x8;
	
	switch (fmt & 0x7) {
		default:  return 'e';
		case 0x1: return 'f';
		case 0x2: return 'g';
		case 0x3: return 'a';
		case 0x4: return 'E';
		case 0x5: return 'F';
		case 0x6: return 'G';
		case 0x7: return 'A';
	}
}

extern int mpt_outfmt_set(const char **pos)
{
	const char *src;
	char *next;
	long val;
	short width;
	char dec, fmt;
	
	if (!pos || !(src = *pos)) {
		errno = EFAULT; return -1;
	}
	while (isspace(*src)) {
		src++;
	}
	/* select format */
	switch (*src) {
		case 0:
			*pos = src;
			return 0;
		case '-':
		case 'n':
		case 'N': fmt = -1;  ++src; break;
		case 'e': fmt = 0x0; ++src; break;
		case 'E': fmt = 0x4; ++src; break;
		case 'f': fmt = 0x1; ++src; break;
		case 'F': fmt = 0x5; ++src; break;
		case 'g': fmt = 0x2; ++src; break;
		case 'G': fmt = 0x6; ++src; break;
		case 'a': fmt = 0x3; ++src; break;
		case 'A': fmt = 0x7; ++src; break;
		default: fmt = 0x0; if (isdigit(*src)) break;
			errno = EINVAL; return -1;
	}
	/* get field with */
	val = strtol(src, &next, 0);
	
	if (next == src) {
		if (fmt < 0) {
			*pos = next;
			return 0;
		}
		return -2;
	}
	if (val < 0)
		return -2;
	
	if (!*next || isspace(*next)) {
		*pos = next;
		return (fmt < 0) ? (0x8000 | MPT_outfmt_fset(val, 6, 0)) : MPT_outfmt_fset(val, 6, fmt);
	}
	if (*(next++) != '.') {
		errno = EINVAL; return -3;
	}
	
	width = val ? ((val > 0x7f) ? 0x7f : val) : 1;
	
	/* get digits after decimal point */
	val = strtol(next, &next, 0);
	
	dec = (val < 0) ? 0 : ((val > 0x1f) ? 0x1f : val);
	
	*pos = next;
	
	return (fmt < 0) ? (0x8000 | MPT_outfmt_fset(width, dec, 0)) : MPT_outfmt_fset(width, dec, fmt);
}

extern int16_t *mpt_outfmt_parse(const char *base, char **endp)
{
	int curr, len = 0, max = 0;
	int16_t *dest = 0;
	
	while ((curr = mpt_outfmt_set(&base)) > 0) {
		if (len >= max) {
			int16_t *next = realloc(dest, max+=32 * sizeof(*dest));
			if (!next) {
				if (endp) *endp = 0;
				free(dest); return 0;
			}
			dest = next;
		}
		dest[len++] = curr;
	}
	if (endp) *endp = (char *) base;
	
	if (!curr) return dest;
	
	free(dest);
	return 0;
}
