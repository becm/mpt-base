
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include "config.h"
#include "parse.h"

/*!
 * \ingroup mptParse
 * \brief parse option data
 * 
 * Parse data part of element.
 * 
 * \param parse parser source data
 * \param path  target path data
 * 
 * \return parser code for next element
 */
extern int mpt_parse_data(const MPT_STRUCT(parser_format) *fmt, MPT_STRUCT(parser_context) *parse, MPT_STRUCT(path) *path)
{
	int curr, match = 0, last = -1;
	
	while ((curr = mpt_parse_getchar(&parse->src, path)) >= 0) {
		/* escape release and begin conditions */
		if (match) {
			if (curr == match) {
				mpt_path_delchar(path);
				if (last != '\\') {
					match = 0;
				} else {
					mpt_path_delchar(path);
					mpt_path_addchar(path, curr);
				}
			}
			parse->valid = mpt_path_valid(path);
		}
		else if (MPT_isescape(fmt, curr)) {
			match = curr;
		}
		/* option end detected */
		else if (curr == fmt->oend) {
			break;
		}
		/* unescaped newline */
		else if (curr == '\n') {
			break;
		}
		/* comments: continue to line end without save */
		else if (!fmt->oend && MPT_iscomment(fmt, curr) && isspace(last)) {
			(void) mpt_parse_endline(&parse->src);
			break;
		}
		/* add currend trailing data to valid area */
		else if (!isspace(curr)) {
			parse->valid = mpt_path_valid(path);
		}
		last = curr;
	}
	
	if (fmt->oend && curr != fmt->oend) {
		parse->curr = MPT_PARSEFLAG(Data);
		return MPT_ERROR(BadValue);
	}
	return parse->valid;
}
