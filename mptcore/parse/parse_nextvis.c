/*!
 * read until 'visible' character is found.
 */

#include <string.h>
#include <ctype.h>

#include "parse.h"

extern int mpt_parse_nextvis(MPT_STRUCT(parser_input) *src, const void *com, size_t len)
{
	int curr;
	
	if (!src->line) ++src->line;
	
	while ((curr = src->getc(src->arg)) > 0) {
		if (curr == '\n') {
			src->line++;
		}
		if (isspace(curr)) {
			continue;
		}
		if (len && memchr(com, curr, len)) {
			if ((curr = mpt_parse_endline(src)) < 0) {
				return curr;
			}
			continue;
		}
		break;
	}
	return curr;
}
