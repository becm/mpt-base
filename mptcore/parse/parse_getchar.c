
#include "config.h"
#include "parse.h"

/*!
 * \ingroup mptParse
 * \brief get next character
 * 
 * get next character from parser source and save to path.
 * 
 * \param parse	parser data
 * \param path	target path
 * 
 * \return new input character
 */
extern int mpt_parse_getchar(MPT_STRUCT(parse) *parse, MPT_STRUCT(path) *path)
{
	int curr;
	
	if ((curr = parse->source.getc(parse->source.arg)) <= 0) {
		return curr;
	}
	if (curr == '\n') {
		parse->line++;
	}
	if (path && mpt_path_addchar(path, curr) < 0) {
		return -1;
	}
	return curr;
}

