/*!
 * get element in array
 */

#include <string.h>

#include "array.h"

/*!
 * \ingroup mptArray
 * \brief get array element
 * 
 * Get address of valid array element
 * assuming elements are of specified size.
 * 
 * \param arr  data array
 * \param pos  element in array
 * \param len  size of single element
 * 
 * \return start address of array element
 */
extern int mpt_buffer_copy(MPT_STRUCT(buffer) *dest, const MPT_STRUCT(buffer) *src)
{
	const MPT_STRUCT(type_traits) *info, *srcinfo;
	int (*copy)(void *, int , void *);
	void (*fini)(void *);
	uint8_t *ptr;
	size_t used, size, len;
	int type;
	
	if (!(info = dest->_typeinfo)) {
		if (!src) {
			dest->_used = 0;
			return 0;
		}
		if (src->_typeinfo) {
			return MPT_ERROR(BadArgument);
		}
		used = dest->_size;
		if (used > dest->_used) {
			used = dest->_used;
		}
		if (used) {
			memcpy(dest + 1, src + 1, used);
		}
		dest->_used = used;
		return 0;
	}
	if (!(type = info->type)
	    || !(size = info->size)) {
		return MPT_ERROR(BadArgument);
	}
	if (!src) {
		used = 0;
	}
	else if (!(srcinfo = src->_typeinfo)
	         || srcinfo->type != type
		 || srcinfo->size != size) {
		return MPT_ERROR(BadType);
	}
	else {
		used = src->_used;
		if (used > dest->_size) {
			used = dest->_size;
		}
		used -= used % size;
	}
	/* target data parameters */
	len = dest->_used;
	len -= len % size;
	ptr = (void *) (dest + 1);
	
	/* skip copy */
	if (!len) {
		;
	}
	/* generic data copy */
	else if (!(copy = info->copy)) {
		/* non default init withut copy forbidden */
		if (info->init) {
			return MPT_ERROR(BadType);
		}
		memcpy(ptr, src + 1, used);
	}
	/* prepare target and copy data */
	else {
		void (*init)(const MPT_STRUCT(type_traits) *, void *);
		uint8_t *from = (void *)(src + 1);
		size_t off;
		
		/* generic init */
		if ((init = info->init)) {
			if (used > len) {
				memset(ptr + len, 0, used - len);
			}
		}
		/* extended init operation */
		else {
			for (off = len; off < used; off += size) {
				init(info, ptr + off);
			}
		}
		/* copy values */
		for (off = 0; off < used; off += size) {
			int pos;
			if ((pos = copy(ptr + off, type, from + 1)) < 0) {
				used = off;
				break;
			}
		}
	}
	/* clear extra data */
	if ((fini = info->fini)) {
		size_t off;
		for (off = used; off < len; off += size) {
			fini(ptr + off);
		}
	}
	dest->_used = used;
	
	return used / size;
}
