
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
extern int mpt_parse_format_pre(const MPT_STRUCT(parsefmt) *fmt, MPT_STRUCT(parse) *parse, MPT_STRUCT(path) *path)
{
	int curr;
	
	/* get next visible character, no save */
	if ((curr = mpt_parse_nextvis(&parse->src, fmt->com, sizeof(fmt->com))) < 0) {
		if (!path->len) {
			return 0;
		}
		return MPT_ERROR(MissingData);
	}
	if (curr == fmt->sstart) {
		parse->curr = MPT_ENUM(ParseSection);
		if (mpt_parse_ncheck(path->base + path->off + path->len, path->valid, parse->name.sect) < 0) {
			return MPT_ERROR(BadType);
		}
		if (mpt_path_add(path) < 0) {
			return MPT_ERROR(BadOperation);
		}
		return MPT_ENUM(ParseSection);
	}
	parse->curr = MPT_ENUM(ParseName);
	if (mpt_path_addchar(path, curr) < 0) {
		return MPT_ERROR(MissingBuffer);
	}
	do {
		if (curr < 0) {
			return MPT_ERROR(MissingData);
		}
		/* found section end */
		if (curr == fmt->send) {
			return parse->curr = MPT_ENUM(ParseSectEnd);
		}
		/* found section start */
		if (curr == fmt->sstart) {
			break;
		}
		/* parse option name */
		if (curr == fmt->ostart) {
			return mpt_parse_option(fmt, parse, path);
		}
		/* parse option data */
		if (curr == fmt->assign) {
			parse->curr = MPT_ENUM(ParseOption) | MPT_ENUM(ParseName);
			if (mpt_parse_ncheck(path->base + path->off + path->len, path->valid, parse->name.opt) < 0) {
				return MPT_ERROR(BadType);
			}
			if (mpt_path_add(path) < 0) {
				return MPT_ERROR(BadOperation);
			}
			/* clear trailing path data */
			mpt_path_invalidate(path);
			
			if ((curr = mpt_parse_data(fmt, parse, path)) < 0) {
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
		parse->curr = MPT_ENUM(ParseName);
		
		/* validate current length character */
		if (!isspace(curr)) {
			mpt_path_valid(path);
		}
		/* get next visible character, no save */
		else if (curr == '\n') {
			curr = mpt_parse_nextvis(&parse->src, fmt->com, sizeof(fmt->com));
			if (mpt_path_addchar(path, curr) < 0) {
				return MPT_ERROR(MissingBuffer);
			}
			break;
		}
	} while ((curr = mpt_parse_getchar(&parse->src, path)) >= 0);
	
	/* last chance to get section start */
	if (fmt->sstart && curr == fmt->sstart) {
		parse->curr = MPT_ENUM(ParseSection) | MPT_ENUM(ParseName);
		if (mpt_parse_ncheck(path->base + path->off + path->len, path->valid, parse->name.sect) < 0) {
			return MPT_ERROR(BadType);
		}
		if (mpt_path_add(path) < 0) {
			return MPT_ERROR(BadOperation);
		}
		return MPT_ENUM(ParseSection);
	}
	/* try to pass as data */
	if (fmt->oend && curr == fmt->oend) {
		parse->curr = MPT_ENUM(ParseData);
		/* set zero length option name */
		if (mpt_parse_ncheck(path->base + path->off + path->len, 0, parse->name.opt) < 0) {
			path->valid = curr;
			return MPT_ERROR(BadType);
		}
		return MPT_ENUM(ParseData);
	}
	parse->curr = MPT_ENUM(ParseName);
	return MPT_ERROR(BadValue);
}

