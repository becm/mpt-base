/*!
 * MPT core library
 *   get config element
 */

#include <string.h>

#include "meta.h"
#include "types.h"

#include "config.h"

struct _concert_value_data
{
	void *ptr;
	MPT_TYPE(type) type;
};

static int _convert_value(void *ptr, MPT_INTERFACE(convertable) *val, const MPT_INTERFACE(collection) *coll)
{
	struct _concert_value_data *ctx = ptr;
	(void) coll;
	if (!val) {
		return MPT_ERROR(MissingData);
	}
	return val->_vptr->convert(val, ctx->type, ctx->ptr);
}

/*!
 * \ingroup mptConfig
 * \brief get configuration element value
 * 
 * Find element in configuration and convert value.
 * 
 * \param conf configuration contxt
 * \param path element path
 * \param cb   callback method
 * \param ctx  callback context
 * 
 * \return config element if exists
 */
extern int mpt_config_query(const MPT_INTERFACE(config) *conf, const MPT_STRUCT(path) *path, MPT_TYPE(config_handler) cb, void *ctx)
{
	if (!conf) {
		MPT_INTERFACE(metatype) *mt;
		
		if (!(mt = mpt_config_global(0))
		 || (MPT_metatype_convert(mt, MPT_ENUM(TypeConfigPtr), &conf) < 0)
		 || !conf) {
			return 0;
		}
	}
	return conf->_vptr->query(conf, path, cb, ctx);
}

/*!
 * \ingroup mptConfig
 * \brief get configuration element value
 * 
 * Find element in configuration and convert value.
 * 
 * \param conf configuration contxt
 * \param path element path
 * \param type target value type
 * \param ptr  target value address
 * 
 * \return config element if exists
 */
extern int mpt_config_getp(const MPT_INTERFACE(config) *conf, const MPT_STRUCT(path) *path, MPT_TYPE(type) type, void *ptr)
{
	struct _concert_value_data ctx = { ptr, type };
	return mpt_config_query(conf, path, type ? _convert_value : 0, &ctx);
}

/*!
 * \ingroup mptConfig
 * \brief get configuration element value
 * 
 * Find element in configuration and convert value.
 * 
 * \param conf configuration contxt
 * \param path element path
 * \param type target value type
 * \param ptr  target value address
 * 
 * \return config element if exists
 */
extern int mpt_config_get(const MPT_INTERFACE(config) *conf, const char *dest, MPT_TYPE(type) type, void *ptr)
{
	MPT_STRUCT(path) path = MPT_PATH_INIT;
	if (dest) {
		path.sep = '.';
		path.assign = 0;
		mpt_path_set(&path, dest, -1);
	}
	return mpt_config_getp(conf, &path, type, ptr);
}
