/*!
 * set point data from metatype.
 */

#include "meta.h"
#include "types.h"

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
extern int mpt_fpoint_set(MPT_STRUCT(fpoint) *pt, MPT_INTERFACE(convertable) *src, const MPT_STRUCT(range) *r)
{
	MPT_INTERFACE(iterator) *it;
	MPT_STRUCT(fpoint) tmp = { 0.0, 0.0 };
	int ret;
	
	if (!src) {
		*pt = tmp;
		return 0;
	}
	it = 0;
	if ((ret = src->_vptr->convert(src, MPT_ENUM(TypeIteratorPtr), &it)) < 0) {
		if ((ret = src->_vptr->convert(src, MPT_ENUM(TypeFloatPoint), &tmp)) < 0) {
			return MPT_ERROR(BadType);
		}
	}
	else {
		if ((ret = it->_vptr->get(it, 'f', &tmp.x)) < 0) {
			return MPT_ERROR(BadType);
		}
		if (!ret) {
			*pt = tmp;
			return 0;
		}
		else if ((ret = it->_vptr->advance(it)) < 0) {
			return ret;
		}
		else if ((ret = it->_vptr->get(it, 'f', &tmp.y)) < 0) {
			return ret;
		}
		else if (!ret) {
			tmp.y = tmp.x;
			ret = 1;
		}
		else if ((ret = it->_vptr->advance(it)) < 0) {
			return ret;
		}
		else {
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
