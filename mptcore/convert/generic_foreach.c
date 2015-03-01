/*!
 * call function for all object/metatype properties
 */

#include "node.h"
#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief process properties
 * 
 * Get properties from element via first
 * property handler and process with second.
 * 
 * \param obj  generic data
 * \param get  get indexed properties form obj
 * \param proc process retreived properties
 * \param data argumernt for property processing
 * \param mask ignore default/changed properties
 * 
 * \return index of requested property
 */
extern int mpt_generic_foreach(MPT_TYPE(PropertyHandler) get, void *obj, MPT_TYPE(PropertyHandler) proc, void *data, int mask)
{
	MPT_STRUCT(property) prop;
	uintptr_t pos = 0;
	int err;
	
	prop.name = 0;
	prop.desc = 0;
	
	while ((err = get(obj, &prop)) >= 0) {
		prop.desc = 0;
		if (((err && !(mask & MPT_ENUM(TraverseChange)))
		     || (!err && !(mask & MPT_ENUM(TraverseDefault))))
		    && (err = proc(data, &prop)) < 0)
			return err;
		
		prop.name = 0;
		prop.desc = (void *)(++pos);
	}
	return pos - 1;
}

static int getProperty(MPT_INTERFACE(metatype) *meta, MPT_STRUCT(property) *pr)
{ return meta->_vptr->property(meta, pr, 0); }

/*!
 * \ingroup mptConvert
 * \brief process properties
 * 
 * Process properties of metatype.
 * 
 * \param meta property source
 * \param proc process retreived properties
 * \param data argumernt for property processing
 * \param mask ignore default/changed properties
 * 
 * \return index of requested property
 */
extern int mpt_meta_foreach(MPT_INTERFACE(metatype) *meta, MPT_TYPE(PropertyHandler) proc, void *data, int mask)
{
	return mpt_generic_foreach((MPT_TYPE(PropertyHandler)) getProperty, (void *) meta, proc, data, mask);
}
