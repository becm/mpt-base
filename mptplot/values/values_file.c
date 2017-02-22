/*!
 * read values from file
 */

#include <stdio.h>
#include <limits.h>
#include <errno.h>

#include "values.h"

/*!
 * \ingroup mptValues
 * \brief read data file
 * 
 * Read double values from file.
 * 
 * Align in column order for len>0, row order for len<0.
 * 
 * \param fd   input file
 * \param len  number of lines
 * \param ld   elements per line
 * \param data target address
 * 
 * \return zero on success
 */
extern int mpt_values_file(FILE *fd, int len, int ld, double *data)
{
	double val;
	int i, j;
	
	/* read aligned columns */
	if (len < 0) {
		len = -len;
		for (i = 0; i < len; i++) {
			for (j = 0; j < ld; j++) {
				if (fscanf(fd, "%lf", data ? &data[len*j] : &val) != 1) {
					return -(j+i*ld);
				}
			}
			if ((fscanf(fd, "%*[^\n]") < 0) && (i+1 < len)) {
				return -((i+1)*ld);
			}
			++data;
		}
	}
	/* read aligned rows */
	else {
		for (i = 0; i < len; i++) {
			for (j = 0; j < ld; j++) {
				if (fscanf(fd, "%lf", data ? &data[j] : &val) != 1) {
					return -(j+i*ld);
				}
			}
			if ((fscanf(fd, "%*[^\n]") < 0) && (i+1 < len)) {
				return -((i+1)*ld);
			}
			data += ld;
		}
	}
	return 0;
}
