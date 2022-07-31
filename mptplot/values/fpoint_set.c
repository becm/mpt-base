/*!
 * set point data from metatype.
 */

#include "meta.h"
#include "types.h"

#include "values.h"

/*!
 * \ingroup mptPlot
 * \brief get or register float 2D point type
 * 
 * Allocate type for float 2D point.
 * 
 * \return ID for type in default namespace
 */
extern int mpt_fpoint_typeid(void)
{
	static int ptype = 0;
	int type;
	if (!(type = ptype)) {
		static const MPT_STRUCT(type_traits) traits = MPT_TYPETRAIT_INIT(sizeof(MPT_STRUCT(fpoint)));
		if ((type = mpt_type_add(&traits)) > 0) {
			ptype = type;
		}
	}
	return type;
}

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
		int type = mpt_fpoint_typeid();
		if (type <= 0 || (ret = src->_vptr->convert(src, type, &tmp)) < 0) {
			return MPT_ERROR(BadType);
		}
	}
	else {
		/* first coordinate */
		if ((ret = mpt_iterator_consume(it, 'f', &tmp.x)) < 0) {
			return ret;
		}
		if (!ret) {
			ret = 1;
			tmp.y = tmp.x;
		}
		else if ((ret = mpt_iterator_consume(it, 'f', &tmp.y)) < 0) {
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
