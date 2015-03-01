

#include "core.h"

/* get metatype name */
/*!
 * \ingroup mptMeta
 * \brief metatype name
 * 
 * Get type name for metatype.
 * 
 * \param mt  MPT metatype pointer
 * 
 * \return name of metatype
 */
extern const char *mpt_meta_typename(MPT_INTERFACE(metatype) *mt)
{
	MPT_STRUCT(property) pr;
	
	pr.name = "";
	pr.desc = 0;
	
	if (mt->_vptr->property(mt, &pr, 0) < 0) {
		return 0;
	}
	return pr.name;
}
