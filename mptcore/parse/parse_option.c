
#include <errno.h>
#include <string.h>

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
extern int mpt_parse_option(MPT_STRUCT(parse) *parse, MPT_STRUCT(path) *path)
{
	const MPT_STRUCT(parsefmt) *fmt = &parse->format;
	int curr;
	
	/* get next visible character, no save */
	if ((curr = mpt_parse_nextvis(&parse->src, fmt->com, sizeof(fmt->com))) < 0) {
		return path->len ? curr : 0;
	}
	if (fmt->ostart && curr != fmt->ostart && path->valid){
		return -MPT_ENUM(ParseOption);
	}
	if (mpt_path_addchar(path, curr) < 0) {
		return -MPT_ENUM(ParseInternal);
	}
	while (1) {
		/* option assign condition */
		if (isspace(curr)) {
			if (!fmt->assign) {
				if (parse->check.ctl &&
				    parse->check.ctl(parse->check.arg, path, MPT_ENUM(ParseOption)) < 0) {
					return -MPT_ENUM(ParseOptName);
				}
				if (mpt_path_add(path) < 0) {
					return -MPT_ENUM(ParseOption);
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
			if (curr == '\n') {
				if (fmt->oend && curr != fmt->oend) {
					return -MPT_ENUM(ParseOptName);
				}
				break;
			}
		}
		else if (curr == fmt->assign) {
			if (parse->check.ctl &&
			    parse->check.ctl(parse->check.arg, path, MPT_ENUM(ParseOption)) < 0) {
				return -MPT_ENUM(ParseOptName);
			}
			if (mpt_path_add(path) < 0) {
				return -MPT_ENUM(ParseOption);
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
		/* option end detected */
		else if (curr == fmt->oend) {
			break;
		}
		/* comments: continue to line end without save */
		else if (MPT_iscomment(fmt, curr)) {
			if (fmt->oend) {
				return -MPT_ENUM(ParseOptName);
			}
			(void) mpt_parse_endline(&parse->src);
			break;
		}
		else {
			mpt_path_valid(path);
		}
		if ((curr = mpt_parse_getchar(&parse->src, path)) < 0) {
			return -MPT_ENUM(ParseOption);
		}
	}
	/* set zero length option name */
	if (parse->check.ctl &&
	    parse->check.ctl(parse->check.arg, path, MPT_ENUM(ParseData)) < 0) {
		return -MPT_ENUM(ParseOptName);
	}
	return MPT_ENUM(ParseData);
}
