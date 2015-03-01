
#include <errno.h>

#include "node.h"
#include "config.h"
#include "parse.h"

/*!
 * \ingroup mptParse
 * \brief read configuration
 * 
 * create or insert elements from configuration from file.
 * 
 * \param next  funtion to get next path element
 * \param parse input information
 * \param root  configuration target
 * 
 * \return error code
 */
extern int mpt_parse_config(MPT_TYPE(ParserFcn) next, MPT_STRUCT(parse) *parse, MPT_STRUCT(node) *root)
{
	MPT_STRUCT(node) *curr, *(*save)(MPT_STRUCT(node) *, const MPT_STRUCT(path) *, int, int);
	MPT_STRUCT(path) path = MPT_PATH_INIT;
	int ret;
	
	if (!(curr = root->children)) {
		curr = root;
		save = mpt_parse_append;
		parse->lastop = MPT_ENUM(ParseSection);
	} else {
		save = (MPT_STRUCT(node) *(*)()) mpt_parse_insert;
		parse->lastop = 0;
	}
	/* accuire next path element */
	while ((ret = next(parse, &path)) > 0) {
		MPT_STRUCT(node) *tmp;
		
		/* save to configuration */
		if (!(tmp = save(curr, &path, parse->lastop, ret))) {
			ret = -0x80;
			break;
		}
		/* remove last path element */
		if (ret & MPT_ENUM(ParseSectEnd)) {
			mpt_path_del(&path);
		}
		mpt_path_invalidate(&path);
		curr = tmp;
		parse->lastop = ret;
	}
	mpt_path_fini(&path);
	
	return ret;
}

