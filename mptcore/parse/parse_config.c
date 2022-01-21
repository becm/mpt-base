
#include <sys/uio.h>

#include "config.h"
#include "types.h"

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
extern int mpt_parse_config(MPT_TYPE(input_parser) next, void *npar, MPT_STRUCT(parser_context) *parse, MPT_TYPE(path_handler) save, void *ctx)
{
	MPT_STRUCT(path) path = MPT_PATH_INIT;
	struct iovec vec;
	MPT_STRUCT(value) val = MPT_VALUE_INIT(MPT_type_toVector('c'), &vec);
	int ret;
	
	/* accuire next path element */
	while ((ret = next(npar, parse, &path)) > 0) {
		vec.iov_base = (char *) (path.base + path.off + path.len);
		vec.iov_len  = parse->valid;
		
		/* save to configuration */
		if (save(ctx, &path, ret & MPT_PARSEFLAG(Data) ? &val : 0, parse->prev, ret) < 0) {
			ret = -0x80;
			break;
		}
		/* remove last path element or trailing data */
		if (ret & MPT_PARSEFLAG(SectEnd)) {
			ret = mpt_path_del(&path);
		} else {
			ret = mpt_path_invalidate(&path);
		}
		if (ret < 0) {
			ret = MPT_ERROR(MissingData);
			break;
		}
		/* cycle parse state */
		parse->prev = parse->curr;
		parse->curr = 0;
		
		/* reset valid post path size */
		parse->valid = 0;
	}
	mpt_path_fini(&path);
	
	return ret;
}
