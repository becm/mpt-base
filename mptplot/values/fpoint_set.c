/*!
 * set point data from metatype.
 */

#include "meta.h"

#include "layout.h"

/*!
 * \ingroup mptPlot
 * \brief set float point
 * 
 * Change or reset point structure properties.
 * 
 * \param pt   float data point
 * \param src  value source
 * \param def  default point value
 */
extern int mpt_fpoint_set(MPT_STRUCT(fpoint) *pt, const MPT_INTERFACE(metatype) *src, const MPT_STRUCT(fpoint) *def, const MPT_STRUCT(range) *r)
{
	MPT_INTERFACE(iterator) *it;
	MPT_STRUCT(fpoint) tmp;
	int ret;
	
	if (!src) {
		*pt = *def;
		return 0;
	}
	it = 0;
	if ((ret = src->_vptr->conv(src, MPT_ENUM(TypeIterator), &it)) < 0) {
		MPT_STRUCT(value) val = MPT_VALUE_INIT;
		
		if ((ret = src->_vptr->conv(src, MPT_ENUM(TypeValue), &val)) < 0) {
			return MPT_ERROR(BadType);
		}
		if (!val.ptr || !val.fmt || !*val.fmt) {
			return MPT_ERROR(BadType);
		}
		if (val.fmt[0] == 'f' && val.fmt[1] == 'f') {
			const MPT_STRUCT(fpoint) *ptr = val.ptr;
			tmp.x = ptr->x;
			tmp.y = ptr->y;
		}
		else if (val.fmt[0] == 'd' && val.fmt[1] == 'd') {
			const MPT_STRUCT(dpoint) *ptr = val.ptr;
			tmp.x = ptr->x;
			tmp.y = ptr->y;
		}
		else {
			return MPT_ERROR(BadType);
		}
		ret = 0;
	}
	else {
		if ((ret = it->_vptr->meta.conv((void *) it, 'f', &tmp.x)) < 0) {
			return MPT_ERROR(BadType);
		}
		if (!ret) {
			tmp = *def;
			ret = 0;
		}
		else if ((ret = it->_vptr->advance(it)) < 0) {
			return ret;
		}
		else if ((ret = it->_vptr->meta.conv((void *) it, 'f', &tmp.y)) < 0) {
			return ret;
		}
		else if (!ret) {
			tmp.y = tmp.x;
			ret = 1;
		} else {
			ret = 2;
		}
	}
	if (r) {
		if (tmp.x < r->min || tmp.y < r->min
		    || tmp.x > r->max || tmp.y > r->max) {
			return MPT_ERROR(BadValue);
		}
	}
	*pt = tmp;
	return ret;
}