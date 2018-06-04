/*!
 * MPT core library
 *   copy buffer elements
 */

#include <string.h>

#include "array.h"

/*!
 * \ingroup mptArray
 * \brief copy buffer content
 * 
 * Replace target buffer content with data from source buffer elements.
 * Elements not fitting into target space are ignored.
 * If element copy is unsuccessful, old (or default) element value is used.
 * 
 * If no copy operator is defined in source,
 * data types are required to be a exact match.
 * 
 * \param dest  data array
 * \param src   element in array
 * 
 * \return start address of array element
 */
extern long mpt_buffer_copy(MPT_STRUCT(buffer) *dest, const MPT_STRUCT(buffer) *src)
{
	void (*init)(const MPT_STRUCT(type_traits) *, void *);
	const MPT_STRUCT(type_traits) *info;
	int (*copy)(void *, int , void *);
	void (*fini)(void *);
	uint8_t *ptr;
	size_t dest_size, src_size;
	size_t used;
	long elem;
	int type;
	
	/* copy raw data */
	if (!(info = dest->_typeinfo)) {
		size_t len;
		if (!src) {
			dest->_used = 0;
			return 0;
		}
		if (src->_typeinfo) {
			return MPT_ERROR(BadArgument);
		}
		len = dest->_size;
		used = src->_used;
		if (used < len) {
			used = len;
		}
		if (used) {
			memcpy(dest + 1, src + 1, used);
		}
		dest->_used = used;
		return 0;
	}
	if (!(type = info->type)) {
		return MPT_ERROR(BadArgument);
	}
	used = dest->_used;
	init = info->init;
	fini = info->fini;
	
	if (!(dest_size = info->size)
	    || used % dest_size) {
		return MPT_ERROR(BadValue);
	}
	if (!src) {
		elem = 0;
	}
	else if (!(info = src->_typeinfo)) {
		return MPT_ERROR(BadArgument);
	}
	else {
		long avail;
		avail = src->_used;
		if (!(src_size = info->size)
		    || avail % src_size) {
			return MPT_ERROR(BadArgument);
		}
		if ((copy = info->copy)) {
			if (!info->init) {
				return MPT_ERROR(BadArgument);
			}
		}
		/* same type for generic copy */
		else if (type != info->type
		         || dest_size != src_size) {
			return MPT_ERROR(BadType);
		}
		/* use element count */
		avail /= src_size;
		/* maximum target elements */
		elem = dest->_size / dest_size;
		/* possible copy locations */
		if (avail < elem) {
			elem = avail;
		}
	}
	/* target data parameters */
	used = dest->_used;
	ptr = (void *) (dest + 1);
	
	/* terminate target data */
	if (!elem) {
		dest->_used = 0;
		if (fini) {
			size_t off;
			for (off = 0; off < used; off += dest_size) {
				fini(ptr + off);
			}
		}
		return 0;
	}
	/* generic data copy, types are guaranteed to be equal (see above) */
	if (!copy) {
		size_t max = elem * dest_size;
		memcpy(ptr, src + 1, max);
		if (used < max) {
			used = max;
		}
	}
	/* prepare target and copy data */
	else {
		uint8_t *from = (void *)(src + 1);
		size_t max = elem * dest_size;
		long success = 0;
		
		/* generic init */
		if (!init) {
			if (used < max) {
				memset(ptr + used, 0, max - used);
			}
		}
		/* extended init operation */
		else {
			size_t off;
			used -= used % dest_size;
			for (off = used; off < max; off += dest_size) {
				init(info, ptr + off);
			}
		}
		/* copy available values */
		while (elem) {
			int pos = copy(from, type, ptr);
			from += src_size;
			ptr += dest_size;
			if (pos >= 0) {
				++success;
			}
		}
		/* grow target size */
		if (used < max) {
			used = max;
		}
		elem = success;
	}
	dest->_used = used;
	
	return elem;
}
