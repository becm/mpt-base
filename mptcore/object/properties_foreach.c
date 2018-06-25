/*!
 * call function for all object/metatype properties
 */

#include "object.h"

/*!
 * \ingroup mptCore
 * \brief process properties
 * 
 * Get properties from element via first
 * property handler and process with second.
 * 
 * \param obj   generic data
 * \param get   get indexed properties form obj
 * \param proc  process retreived properties
 * \param data  argumernt for property processing
 * \param match traverse flags for changed or default properties
 * 
 * \return index of requested property
 */
extern int mpt_properties_foreach(int (*get)(void *, MPT_STRUCT(property) *), void *obj, MPT_TYPE(property_handler) proc, void *data, int match)
{
	MPT_STRUCT(property) prop;
	uintptr_t pos = 0;
	int err;
	
	prop.name = 0;
	prop.desc = 0;
	
	while ((err = get(obj, &prop)) >= 0) {
		prop.desc = 0;
		if (((err && (match & MPT_ENUM(TraverseChange)))
		     || (!err && (match & MPT_ENUM(TraverseDefault))))
		    && (err = proc(data, &prop)) < 0) {
			return err;
		}
		prop.name = 0;
		prop.desc = (void *)(++pos);
	}
	return pos - 1;
}
