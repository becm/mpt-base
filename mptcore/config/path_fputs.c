
#include <stdio.h>

#include "config.h"

/*!
 * \ingroup mptConfig
 * \brief print path data
 * 
 * print path data to file descriptor.
 * 
 * \param path   path data
 * \param out    C file descriptor
 * \param assign assignment string
 * \param sep    path element separator
 * 
 * \return 0 on succes
 */
extern int mpt_path_fputs(const MPT_STRUCT(path) *path, FILE *out, const char *assign, const char *sep)
{
	MPT_STRUCT(path) tmp;
	const char *base, *curr;
	int clen;
	
	if (!path->len) {
		if ((clen = path->valid) && !fwrite(path->base, clen, 1, out)) {
			return -1;
		}
		return 2;
	}
	tmp  = *path;
	base = tmp.base;
	curr = base + tmp.off;
	
	if (!sep) sep = "/";
	
	/* loop for path elements */
	while ((clen = mpt_path_next(&tmp)) >= 0) {
		fputs(sep, out);
		if (clen && !fwrite(curr, clen, 1, out)) {
			return -1;
		}
		curr = base + tmp.off;
	}
	/* output data part */
	if ((clen = path->valid)) {
		if (assign) {
			fputs(assign, out);
		}
		if (!fwrite(curr, clen, 1, out)) {
			return -1;
		}
		return 3;
	}
	return 1;
}

