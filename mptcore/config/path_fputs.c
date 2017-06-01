
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
 * \param sep    path element separator
 * 
 * \return 0 on succes
 */
extern int mpt_path_fputs(const MPT_STRUCT(path) *path, FILE *out, const char *sep)
{
	MPT_STRUCT(path) tmp;
	const char *base, *curr;
	int clen, elem;
	
	if (!path->len) {
		return 0;
	}
	tmp  = *path;
	base = tmp.base;
	curr = base + tmp.off;
	
	if (!sep) sep = "/";
	
	/* loop for path elements */
	elem = 0;
	while ((clen = mpt_path_next(&tmp)) >= 0) {
		fputs(sep, out);
		if (clen && !fwrite(curr, clen, 1, out)) {
			return -1;
		}
		curr = base + tmp.off;
		++elem;
	}
	return elem;
}

