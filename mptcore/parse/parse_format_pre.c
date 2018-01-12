
#include <string.h>
#include <ctype.h>

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
		parse->curr = MPT_PARSEFLAG(Section);
		if (mpt_parse_ncheck(path->base + path->off + path->len, parse->valid, parse->name.sect) < 0) {
			return MPT_ERROR(BadType);
		}
		if (mpt_path_add(path, parse->valid) < 0) {
			return MPT_ERROR(BadOperation);
		}
		return MPT_PARSEFLAG(Section);
	}
	parse->curr = MPT_PARSEFLAG(Name);
	if (mpt_path_addchar(path, curr) < 0) {
		return MPT_ERROR(MissingBuffer);
	}
	do {
		if (curr < 0) {
			return MPT_ERROR(MissingData);
		}
		/* found section end */
		if (curr == fmt->send) {
			return parse->curr = MPT_PARSEFLAG(SectEnd);
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
			parse->curr = MPT_PARSEFLAG(Option) | MPT_PARSEFLAG(Name);
			if (mpt_parse_ncheck(path->base + path->off + path->len, parse->valid, parse->name.opt) < 0) {
				return MPT_ERROR(BadType);
			}
			if (mpt_path_add(path, parse->valid) < 0) {
				return MPT_ERROR(BadOperation);
			}
			/* clear trailing path data */
			mpt_path_invalidate(path);
			parse->valid = 0;
			
			if ((curr = mpt_parse_data(fmt, parse, path)) < 0) {
				return curr;
			}
			if (!curr) {
				return MPT_PARSEFLAG(Option);
			}
			return MPT_PARSEFLAG(Option) | MPT_PARSEFLAG(Data);
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
		parse->curr = MPT_PARSEFLAG(Name);
		
		/* validate current length character */
		if (!isspace(curr)) {
			parse->valid = mpt_path_valid(path);
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
		parse->curr = MPT_PARSEFLAG(Section) | MPT_PARSEFLAG(Name);
		if (mpt_parse_ncheck(path->base + path->off + path->len, parse->valid, parse->name.sect) < 0) {
			return MPT_ERROR(BadType);
		}
		if (mpt_path_add(path, parse->valid) < 0) {
			return MPT_ERROR(BadOperation);
		}
		return MPT_PARSEFLAG(Section);
	}
	/* try to pass as data */
	if (fmt->oend && curr == fmt->oend) {
		parse->curr = MPT_PARSEFLAG(Data);
		/* set zero length option name */
		if (mpt_parse_ncheck(path->base + path->off + path->len, 0, parse->name.opt) < 0) {
			return MPT_ERROR(BadType);
		}
		return MPT_PARSEFLAG(Data);
	}
	parse->curr = MPT_PARSEFLAG(Name);
	return MPT_ERROR(BadValue);
}

