
#include <string.h>
#include <ctype.h>

#include "config.h"
#include "parse.h"

/*!
 * \ingroup mptParse
 * \brief parse option element
 * 
 * Parse option part of element.
 * 
 * \param parse parser source data
 * \param path  target path data
 * 
 * \return parser code for next element
 */
extern int mpt_parse_option(const MPT_STRUCT(parser_format) *fmt, MPT_STRUCT(parser_context) *parse, MPT_STRUCT(path) *path)
{
	int curr;
	
	/* get next visible character, no save */
	if ((curr = mpt_parse_nextvis(&parse->src, fmt->com, sizeof(fmt->com))) < 0) {
		parse->curr = parse->valid ? (MPT_PARSEFLAG(Option) | MPT_PARSEFLAG(Name)) : MPT_PARSEFLAG(Option);
		if (curr != -2) {
			return MPT_ERROR(BadArgument);
		}
		return path->len ? MPT_ERROR(MissingData) : 0;
	}
	if (fmt->ostart && curr != fmt->ostart && parse->valid){
		parse->curr = MPT_PARSEFLAG(Option) | MPT_PARSEFLAG(Name);
		return MPT_ERROR(BadValue);
	}
	if (mpt_path_addchar(path, curr) < 0) {
		parse->curr = MPT_PARSEFLAG(Option) | MPT_PARSEFLAG(Name);
		return MPT_ERROR(MissingBuffer);
	}
	parse->curr = MPT_PARSEFLAG(Option) | MPT_PARSEFLAG(Name);
	
	while (1) {
		/* option assign condition */
		if (isspace(curr)) {
			if (!fmt->assign) {
				if (mpt_parse_ncheck(path->base + path->off + path->len, parse->valid, MPT_PARSEFLAG(Option)) < 0) {
					return MPT_ERROR(BadType);
				}
				if (mpt_path_add(path, parse->valid) < 0) {
					return MPT_ERROR(MissingBuffer);
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
			if (curr == '\n') {
				if (fmt->oend && curr != fmt->oend) {
					return MPT_ERROR(BadValue);
				}
				break;
			}
		}
		else if (curr == fmt->assign) {
			if (mpt_parse_ncheck(path->base + path->off + path->len, parse->valid, MPT_PARSEFLAG(Option)) < 0) {
				return MPT_ERROR(BadType);
			}
			if (mpt_path_add(path, parse->valid) < 0) {
				return MPT_ERROR(MissingBuffer);
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
		/* option end detected */
		else if (curr == fmt->oend) {
			break;
		}
		/* comments: continue to line end without save */
		else if (MPT_iscomment(fmt, curr)) {
			if (fmt->oend) {
				return MPT_ERROR(BadValue);
			}
			(void) mpt_parse_endline(&parse->src);
			break;
		}
		else {
			parse->valid = mpt_path_valid(path);
		}
		if ((curr = mpt_parse_getchar(&parse->src, path)) < 0) {
			return (curr == -2) ? MPT_ERROR(MissingData) : MPT_ERROR(BadArgument);
		}
	}
	parse->curr = MPT_PARSEFLAG(Data);
	/* set zero length option name */
	if (mpt_parse_ncheck(path->base + path->off + path->len, parse->valid, MPT_PARSEFLAG(Data)) < 0) {
		return MPT_ERROR(BadType);
	}
	return MPT_PARSEFLAG(Data);
}
