
#include <errno.h>
#include <string.h>

#include "config.h"
#include "parse.h"

/*!
 * \ingroup mptParse
 * \brief parse "separating" element
 * 
 * Separator for sections appear before
 * and after section name.
 * Start of next nection indicates end of current
 * so depth is limited to one.
 * 
 * \param parse parser source data
 * \param path  target path data
 * 
 * \return parser code for next element
 */
extern int mpt_parse_format_sep(MPT_STRUCT(parse) *parse, MPT_STRUCT(path) *path)
{
	const MPT_STRUCT(parsefmt) *fmt = &parse->format;
	int curr;
	
	/* last operation was section end */
	if ((parse->lastop & 0xf) == MPT_ENUM(ParseSectEnd)) {
		curr = fmt->sstart;
		/* find section name start */
		if ((fmt->send != fmt->sstart) && ((curr = mpt_parse_getchar(&parse->src, path)) < 0)) {
			MPT_parse_fail(parse, MPT_ENUM(ParseSection));
			return curr == -2 ? MPT_ERROR(MissingData) : MPT_ERROR(BadArgument);
		}
	}
	/* get next visible character, no save */
	else if ((curr = mpt_parse_nextvis(&parse->src, fmt->com, sizeof(fmt->com))) < 0) {
		if (curr == -2) {
			return 0;
		}
		MPT_parse_fail(parse, MPT_ENUM(ParseName));
		return MPT_ERROR(BadArgument);
	}
	/* section start missed */
	else if (curr != fmt->sstart) {
		if (curr != fmt->ostart) {
			if (mpt_path_addchar(path, curr) < 0) {
				MPT_parse_fail(parse, MPT_ENUM(ParseName));
				return MPT_ERROR(BadOperation);
			}
			mpt_path_valid(path);
		}
		return mpt_parse_option(parse, path);
	}
	/* reached path separator */
	else if (path->len) {
		return MPT_ENUM(ParseSectEnd);
	}
	/* get first name element */
	else if ((fmt->send != fmt->sstart) && ((curr = mpt_parse_getchar(&parse->src, path)) < 0)) {
		MPT_parse_fail(parse, MPT_ENUM(ParseSection));
		return curr == -2 ? MPT_ERROR(MissingData) : MPT_ERROR(BadArgument);
	}
	/* find valid section separator */
	while (1) {
		if (curr == fmt->send) {
			if (parse->check.ctl &&
			    parse->check.ctl(parse->check.arg, path, MPT_ENUM(ParseSection)) < 0) {
				MPT_parse_fail(parse, MPT_ENUM(ParseSection) | MPT_ENUM(ParseName));
				return MPT_ERROR(BadType);
			}
			if (mpt_path_add(path) < 0) {
				MPT_parse_fail(parse, MPT_ENUM(ParseSection) | MPT_ENUM(ParseName));
				return MPT_ERROR(BadOperation);
			}
			return MPT_ENUM(ParseSection);
		}
		if (MPT_iscomment(fmt, curr)) {
			break;
		}
		if (!isspace(curr)) {
			mpt_path_valid(path);
		}
		else if (curr == '\n') {
			break;
		}
		if ((curr = mpt_parse_getchar(&parse->src, path)) < 0) {
			break;
		}
	}
	MPT_parse_fail(parse, MPT_ENUM(ParseSection));
	return MPT_ERROR(BadValue);
}

