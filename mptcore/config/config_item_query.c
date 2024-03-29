/*!
 * MPT core library
 *   get config item for path
 */

#include "array.h"

#include "config.h"

/*!
 * \ingroup mptConfig
 * \brief get config item
 * 
 * Get matching configuration item for path.
 * 
 * \param arr   configuration interface
 * \param path  element position
 * 
 * \return configuration item
 */
extern MPT_STRUCT(config_item) *mpt_config_item_query(const _MPT_UARRAY_TYPE(MPT_STRUCT(config_item)) *arr, MPT_STRUCT(path) *path)
{
	const MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(config_item) *item;
	const char *name;
	size_t item_count;
	int len;
	
	name = path->base + path->off;
	if (!(buf = arr->_buf) || (len = mpt_path_next(path)) < 0) {
		return 0;
	}
	if (buf->_content_traits != mpt_config_item_traits()) {
		return 0;
	}
	item_count = buf->_size / sizeof(*item);
	item = (MPT_STRUCT(config_item) *) (buf + 1);
	while (item_count--) {
		if (item->identifier._len == 0
		 || mpt_identifier_compare(&item->identifier, name, len) != 0) {
			++item;
			continue;
		}
		if (path->len) {
			return mpt_config_item_query(&item->elements, path);
		}
		return item;
	}
	return 0;
}
