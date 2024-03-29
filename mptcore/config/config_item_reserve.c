/*!
 * MPT core library
 *   reserve config item for path
 */

#include <errno.h>

#include "array.h"
#include "meta.h"
#include "types.h"

#include "config.h"

/*!
 * \ingroup mptConfig
 * \brief reserve config item
 * 
 * Create or use a matching configuration item for path.
 * 
 * \param arr   configuration interface
 * \param path  element position
 * 
 * \return configuration item
 */
extern MPT_STRUCT(config_item) *mpt_config_item_reserve(_MPT_UARRAY_TYPE(MPT_STRUCT(config_item)) *arr, MPT_STRUCT(path) *path)
{
	const MPT_STRUCT(type_traits) *traits;
	const MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(config_item) *item, *unused;
	const char *name;
	size_t item_count = 0;
	int len;
	
	name = path->base + path->off;
	if ((len = mpt_path_next(path)) < 0) {
		return 0;
	}
	traits = mpt_config_item_traits();
	if ((buf = arr->_buf)) {
		if (buf->_content_traits != traits) {
			errno = EINVAL;
			return 0;
		}
		item_count = buf->_used / sizeof(*item);
	}
	item = (MPT_STRUCT(config_item) *) (buf + 1);
	unused = 0;
	while (item_count--) {
		if (item->identifier._len == 0) {
			if (!unused) {
				unused = item;
			}
			++item;
			continue;
		}
		if (mpt_identifier_compare(&item->identifier, name, len) != 0) {
			++item;
			continue;
		}
		if (path->len) {
			return mpt_config_item_reserve(&item->elements, path);
		}
		return item;
	}
	if (!unused) {
		/* create initial buffer with unique array flags */
		if (!buf) {
			MPT_STRUCT(buffer) *b;
			if (!(b = _mpt_buffer_alloc(sizeof(*item), MPT_ENUM(BufferNoCopy)))) {
				return 0;
			}
			unused = (MPT_STRUCT(config_item) *) (b + 1);
			b->_content_traits = traits;
			b->_used = sizeof(*item);
			arr->_buf = b;
		}
		else if (!(unused = mpt_array_insert(arr, buf->_used, sizeof(*unused)))) {
			return 0;
		}
		traits->init(unused, 0);
	}
	else {
		MPT_INTERFACE(metatype) *old;
		MPT_STRUCT(buffer) *sub;
		if ((old = unused->value)) {
			old->_vptr->unref(old);
			unused->value = 0;
		}
		if ((sub = unused->elements._buf)) {
			mpt_buffer_cut(sub, 0, buf->_used);
			mpt_array_reduce(&unused->elements);
		}
	}
	if (!mpt_identifier_set(&unused->identifier, name, len)) {
		return 0;
	}
	if (path->len) {
		return mpt_config_item_reserve(&unused->elements, path);
	}
	return unused;
}
