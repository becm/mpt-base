
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
extern int mpt_parse_config(MPT_TYPE(ParserFcn) next, void *npar, MPT_STRUCT(parse) *parse, MPT_TYPE(PathHandler) save, void *ctx)
{
	MPT_STRUCT(path) path = MPT_PATH_INIT;
	int ret;
	
	/* accuire next path element */
	while ((ret = next(npar, parse, &path)) > 0) {
		/* save to configuration */
		if (save(ctx, &path, parse->prev, ret) < 0) {
			ret = -0x80;
			break;
		}
		/* remove last path element */
		if (ret & MPT_ENUM(ParseSectEnd)) {
			mpt_path_del(&path);
		}
		mpt_path_invalidate(&path);
		parse->prev = parse->curr;
		parse->curr = 0;
	}
	mpt_path_fini(&path);
	
	return ret;
}

