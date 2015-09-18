
#include <string.h>

#include "config.h"
#include "parse.h"

/*!
 * \ingroup mptParse
 * \brief parse "prepending" element
 * 
 * Separator for sections appear after
 * section name and at section end.
 * 
 * \param parse parser source data
 * \param path  target path data
 * 
 * \return parser code for next element
 */
extern int mpt_parse_format_pre(MPT_STRUCT(parse) *parse, MPT_STRUCT(path) *path)
{
	const MPT_STRUCT(parsefmt) *fmt = &parse->format;
	int curr;
	
	/* get next visible character, no save */
	if ((curr = mpt_parse_nextvis(&parse->src, fmt->com, sizeof(fmt->com))) < 0) {
		return path->len ? -MPT_ENUM(ParseSectEnd) : 0;
	}
	if (mpt_path_addchar(path, curr) < 0) {
		return -MPT_ENUM(ParseInternal);
	}
	do {
		if (curr < 0) {
			return curr;
		}
		/* found section end */
		if (curr == fmt->send) {
			return MPT_ENUM(ParseSectEnd);
		}
		/* found section start */
		if (curr == fmt->sstart) {
			break;
		}
		/* parse option name */
		if (curr == fmt->ostart) {
			return mpt_parse_option(parse, path);
		}
		/* parse option data */
		if (curr == fmt->assign) {
			if (parse->check.ctl &&
			    parse->check.ctl(parse->check.arg, path, MPT_ENUM(ParseOption)) < 0) {
				return -MPT_ENUM(ParseOptName);
			}
			if (mpt_path_add(path) < 0) {
				return -(MPT_ENUM(ParseInternal) | MPT_ENUM(ParseOption));
			}
			/* clear trailing path data */
			mpt_path_invalidate(path);
			
			if ((curr = mpt_parse_data(parse, path)) < 0) {
				return curr;
			}
			if (!curr) {
				return MPT_ENUM(ParseOption);
			}
			return MPT_ENUM(ParseOption) | MPT_ENUM(ParseData);
		}
		/* data end found */
		if (curr == fmt->oend) {
			break;
		}
		/* comments: continue to line end without save */
		if (MPT_iscomment(fmt, curr)) {
			(void) mpt_parse_endline(&parse->src);
			break;
		}
		
		/* validate current length character */
		if (!isspace(curr)) {
			mpt_path_valid(path);
		}
		/* get next visible character, no save */
		else if (curr == '\n') {
			curr = mpt_parse_nextvis(&parse->src, fmt->com, sizeof(fmt->com));
			if (mpt_path_addchar(path, curr) < 0) {
				return -MPT_ENUM(ParseInternal);
			}
			break;
		}
	} while ((curr = mpt_parse_getchar(&parse->src, path)) >= 0);
	
	/* last chance to get section start */
	if (fmt->sstart && curr == fmt->sstart) {
		if (parse->check.ctl &&
		    parse->check.ctl(parse->check.arg, path, MPT_ENUM(ParseSection)) < 0) {
			return -MPT_ENUM(ParseSectName);
		}
		if (mpt_path_add(path) < 0) {
			return -(MPT_ENUM(ParseInternal) | MPT_ENUM(ParseSection));
		}
		return MPT_ENUM(ParseSection);
	}
	/* try to pass as data */
	if (fmt->oend && curr == fmt->oend) {
		/* set zero length option name */
		curr = path->valid;
		path->valid = 0;
		if (parse->check.ctl &&
		    parse->check.ctl(parse->check.arg, path, MPT_ENUM(ParseData)) < 0) {
			path->valid = curr;
			return -MPT_ENUM(ParseOptName);
		}
		path->valid = curr;
		return MPT_ENUM(ParseData);
	}
	return -MPT_ENUM(ParseSectName);
}

