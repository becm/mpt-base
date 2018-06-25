
#include <sys/uio.h>

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
extern int mpt_parse_config(MPT_TYPE(input_parser) next, void *npar, MPT_STRUCT(parser_context) *parse, MPT_TYPE(path_handler) save, void *ctx)
{
	MPT_STRUCT(path) path = MPT_PATH_INIT;
	int ret;
	
	/* accuire next path element */
	while ((ret = next(npar, parse, &path)) > 0) {
		static const uint8_t fmt[] = { MPT_type_vector('c'), 0 };
		MPT_STRUCT(value) val;
		struct iovec vec;
		
		vec.iov_base = (char *) (path.base + path.off + path.len);
		vec.iov_len  = parse->valid;
		
		val.fmt = fmt;
		val.ptr = &vec;
		
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
