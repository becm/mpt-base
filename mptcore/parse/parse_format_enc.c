
#include <string.h>

#include "config.h"
#include "parse.h"

/*!
 * \ingroup mptParse
 * \brief parse "encapsulated" element
 * 
 * Separator for sections appear before
 * section name and at section end.
 * 
 * \param parse parser source data
 * \param path  target path data
 * 
 * \return parser code for next element
 */
extern int mpt_parse_format_enc(MPT_STRUCT(parse) *parse, MPT_STRUCT(path) *path)
{
	const MPT_STRUCT(parsefmt) *fmt = &parse->format;
	int curr;
	
	/* same-character section start/end */
	if (fmt->sstart == fmt->send) {
		if (parse->lastop == MPT_ENUM(ParseSectEnd)) {
			curr = fmt->sstart;
		}
		/* no further data and no section end */
		else if ((curr = mpt_parse_nextvis(&parse->src, fmt->com, sizeof(fmt->com))) < 0) {
			return 0;
		}
		/* section start == end detected */
		else if (path->len && curr == fmt->sstart) {
			return MPT_ENUM(ParseSectEnd);
		}
	}
	/* get next visible character, no save */
	else if ((curr = mpt_parse_nextvis(&parse->src, fmt->com, sizeof(fmt->com))) < 0) {
		if (!path->len && curr == -2) {
			return curr;
		}
		MPT_parse_fail(parse, MPT_ENUM(ParseName));
		return MPT_ERROR(MissingData);
	}
	/* section start missed */
	if (curr != fmt->sstart) {
		if (mpt_path_addchar(path, curr) < 0) {
			MPT_parse_fail(parse, MPT_ENUM(ParseOption));
			return MPT_ERROR(MissingBuffer);
		}
		if (fmt->ostart) {
			if (curr != fmt->ostart) {
				MPT_parse_fail(parse, MPT_ENUM(ParseOption));
				return MPT_ERROR(BadValue);
			}
		}
		else {
			mpt_path_valid(path);
		}
		return mpt_parse_option(parse, path);
	}
	/* first character of section name */
	if ((curr = mpt_parse_nextvis(&parse->src, fmt->com, sizeof(fmt->com))) <= 0) {
		MPT_parse_fail(parse, MPT_ENUM(ParseSection));
		return MPT_ERROR(MissingData);
	}
	
	if (mpt_path_addchar(path, curr) < 0) {
		MPT_parse_fail(parse, MPT_ENUM(ParseSection) | MPT_ENUM(ParseName));
		return MPT_ERROR(MissingBuffer);
	}
	mpt_path_valid(path);
	
	while (1) {
		if ((curr = mpt_parse_getchar(&parse->src, path)) <= 0) {
			MPT_parse_fail(parse, MPT_ENUM(ParseSection) | MPT_ENUM(ParseName));
			return MPT_ERROR(MissingData);
		}
		/* found valid section separator */
		if (isspace(curr)) {
			break;
		}
		if (MPT_iscomment(fmt, curr)) {
			(void) mpt_parse_endline(&parse->src);
			break;
		}
		mpt_path_valid(path);
	}
	
	if (parse->check.ctl
	    && parse->check.ctl(parse->check.arg, path, MPT_ENUM(ParseSection)) < 0) {
		MPT_parse_fail(parse, MPT_ENUM(ParseSection) | MPT_ENUM(ParseName));
		return MPT_ERROR(BadType);
	}
	if (mpt_path_add(path) < 0) {
		MPT_parse_fail(parse, MPT_ENUM(ParseSection) | MPT_ENUM(ParseName));
		return MPT_ERROR(BadOperation);
	}
	return MPT_ENUM(ParseSection);
}

