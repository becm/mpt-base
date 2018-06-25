
#include "parse.h"

/*!
 * \ingroup mptParse
 * \brief continue to line end
 * 
 * Read till end of line without saving.
 * 
 * \param src  parse input data
 * 
 * \return newline character
 */
extern int mpt_parse_endline(MPT_STRUCT(parser_input) *src)
{
	int curr, len;
	
	len = 0;
	if (!src->line) ++src->line;
	
	while ((curr = src->getc(src->arg)) != '\n') {
		if (curr < 0) {
			return curr;
		}
		++len;
	}
	++src->line;
	
	return len;
}

