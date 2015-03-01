/*!
 * read until 'visible' character is found.
 */

#include <string.h>

#include "parse.h"

extern int mpt_parse_nextvis(MPT_STRUCT(parse) *parse, const void *com, size_t len)
{
	int curr;
	
	while ((curr = parse->source.getc(parse->source.arg)) > 0) {
		if (curr == '\n') {
			parse->line++;
		}
		if (isspace(curr))
			continue;
		
		if (len && memchr(com, curr, len)) {
			if ( (curr = mpt_parse_endline(parse)) != '\n' )
				break;
			parse->line++;
			continue;
		}
		break;
	}
	return curr;
}
