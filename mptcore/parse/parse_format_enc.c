
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
extern int mpt_parse_format_enc(const MPT_STRUCT(parsefmt) *fmt, MPT_STRUCT(parse) *parse, MPT_STRUCT(path) *path)
{
	int curr;
	
	/* same-character section start/end */
	if (fmt->sstart == fmt->send) {
		if (parse->prev == MPT_PARSEFLAG(SectEnd)) {
			curr = fmt->sstart;
		}
		/* no further data and no section end */
		else if ((curr = mpt_parse_nextvis(&parse->src, fmt->com, sizeof(fmt->com))) < 0) {
			parse->curr = 0;
			return 0;
		}
		/* section start == end detected */
		else if (path->len && curr == fmt->sstart) {
			return parse->curr = MPT_PARSEFLAG(SectEnd);
		}
	}
	/* get next visible character */
	else if ((curr = mpt_parse_nextvis(&parse->src, fmt->com, sizeof(fmt->com))) < 0) {
		parse->curr = MPT_PARSEFLAG(Name);
		if (!path->len && curr == -2) {
			return curr;
		}
		return MPT_ERROR(MissingData);
	}
	/* section start missed */
	if (curr != fmt->sstart) {
		if (mpt_path_addchar(path, curr) < 0) {
			parse->curr = MPT_PARSEFLAG(Option);
			return MPT_ERROR(MissingBuffer);
		}
		if (fmt->ostart) {
			if (curr != fmt->ostart) {
				parse->curr = MPT_PARSEFLAG(Option);
				return MPT_ERROR(BadValue);
			}
		}
		else {
			parse->valid = mpt_path_valid(path);
		}
		return mpt_parse_option(fmt, parse, path);
	}
	parse->curr = MPT_PARSEFLAG(Section);
	/* first character of section name */
	if ((curr = mpt_parse_nextvis(&parse->src, fmt->com, sizeof(fmt->com))) <= 0) {
		return MPT_ERROR(MissingData);
	}
	parse->curr = MPT_PARSEFLAG(Section) | MPT_PARSEFLAG(Name);
	
	if (mpt_path_addchar(path, curr) < 0) {
		return MPT_ERROR(MissingBuffer);
	}
	parse->valid = mpt_path_valid(path);
	
	while (1) {
		if ((curr = mpt_parse_getchar(&parse->src, path)) <= 0) {
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
		parse->valid = mpt_path_valid(path);
	}
	/* apply simple format limits */
	if (mpt_parse_ncheck(path->base + path->off + path->len, parse->valid, parse->name.sect) < 0) {
		return MPT_ERROR(BadType);
	}
	if (mpt_path_add(path, parse->valid) < 0) {
		return MPT_ERROR(BadOperation);
	}
	parse->valid = 0;
	return MPT_PARSEFLAG(Section);
}

