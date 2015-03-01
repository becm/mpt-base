
#include "parse.h"

/*!
 * \ingroup mptParse
 * \brief continue to line end
 * 
 * Read till end of line without saving.
 * 
 * \param parse parse structure
 * 
 * \return newline character
 */
extern int mpt_parse_endline(MPT_STRUCT(parse) *parse)
{
	int curr;
	
	while ((curr = parse->source.getc(parse->source.arg)) != '\n') {
		if (curr < 0) {
			return curr;
		}
	}
	return curr;
}

