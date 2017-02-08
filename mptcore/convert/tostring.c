/*!
 * print arguments to destination string.
 */

#include <string.h>
#include <stdio.h>

#include "convert.h"

#include "../mptplot/layout.h"

/*!
 * \ingroup mptConvert
 * \brief print data
 * 
 * Add stringifyed data to buffer.
 * 
 * \param val  value format and data
 * \param save function to save text data
 * \param dest save target pointer
 * 
 * \return number of processed elements (0 = all)
 */
extern int mpt_tostring(const MPT_STRUCT(value) *val, ssize_t (*save)(void *, const char *, size_t), void *dest)
{
	const char *fmt = val->fmt;
	const void *data = val->ptr;
	int cont = 0;
	
	if (!fmt) {
		if (data && (cont = save(dest, data, strlen(data))) < 0) {
			return cont;
		}
		return 0;
	}
	if (!data) {
		return MPT_ERROR(MissingData);
	}
	while (*fmt) {
		static const MPT_STRUCT(valfmt) vfmt = MPT_VALFMT_INIT;
		char buf[256];
		const char *txt;
		size_t len;
		int adv, curr;
		
		if ((txt = mpt_data_tostring(&data, MPT_ENUM(ValueConsume) | *fmt, &len))) {
			adv = len;
		}
		else if (*fmt == MPT_ENUM(TypeColor)) {
			const MPT_STRUCT(color) *c = data;
			if (c->alpha != 0xff) {
				adv = snprintf(buf, sizeof(buf), "#%02x%02x%02x%02x", c->red, c->green, c->blue, c->alpha);
			} else {
				adv = snprintf(buf, sizeof(buf), "#%02x%02x%02x", c->red, c->green, c->blue);
			}
			data = c + 1;
			txt = buf;
		}
		else if ((adv = mpt_number_print(buf, sizeof(buf), vfmt, *fmt, data)) < 0) {
			return cont ? cont : adv;
		}
		else {
			data = ((uint8_t *) data) + mpt_valsize(*fmt);
			txt = buf;
		}
		if ((cont && (curr = save(dest, " ", 1)) < 1)
		    || (curr = save(dest, txt, adv)) < adv) {
			return cont ? cont : curr;
		}
		++fmt;
		++cont;
	}
	return 0;
}
