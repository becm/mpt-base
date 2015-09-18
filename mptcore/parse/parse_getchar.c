
#include "config.h"
#include "parse.h"

/*!
 * \ingroup mptParse
 * \brief get next character
 * 
 * get next character from parser source and save to path.
 * 
 * \param src   parser input
 * \param path  target path
 * 
 * \return new input character
 */
extern int mpt_parse_getchar(MPT_STRUCT(parseinput) *src, MPT_STRUCT(path) *path)
{
	int curr;
	
	if (!src->line) ++src->line;
	
	if ((curr = src->getc(src->arg)) <= 0) {
		return curr;
	}
	if (curr == '\n') {
		src->line++;
	}
	if (path && mpt_path_addchar(path, curr) < 0) {
		return -1;
	}
	return curr;
}

