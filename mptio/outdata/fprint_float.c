
#include <errno.h>
#include <stdio.h>

#include "message.h"

/*!
 * \ingroup mptMessage
 * \brief print float values
 * 
 * Print available floating point elements to file.
 * 
 * \param file file descriptor
 * \param str  output format
 * \param src  floating point data source
 */
extern int mpt_fprint_float(FILE *file, const int16_t *str, MPT_INTERFACE(source) *src)
{
	char fmt[] = " %*.*f";
	const char *fs = fmt + 1;
	int width = 0, dec = 6, adv = 'f';
	
	if (!file) {
		errno = EFAULT; return -1;
	}
	while (1) {
		double	val;
		
		if (src->_vptr->conv(src, 'd', &val) < 0) {
			float	small;
			if (src->_vptr->conv(src, 'f', &small) <= 0) {
				break;
			}
			val = small;
		}
		/* update format settings */
		if (str && *str) {
			adv = mpt_outfmt_get(*(str++), &width, &dec);
		}
		if (adv > 0) {
			fmt[5] = adv;
			fprintf(file, fs, width, dec, val);
			fs = fmt;
		}
	}
	return 0;
}

