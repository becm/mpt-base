/*!
 * set profile data from descriptions
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "array.h"
#include "node.h"

#include "values.h"

/*!
 * \ingroup mptValues
 * \brief configure parameter data
 * 
 * Get profile data from configuration list.
 * Use ld<0 for alignment by profile.
 * 
 * \param len	length of profiles
 * \param dest	target address
 * \param ld	number of profiles
 * \param conf	profile configuration node
 * \param grid	grid data (needed for polynom type)
 * 
 * \return zero on success
 */
extern int mpt_conf_profiles(int len, double *dest, int ld, const MPT_STRUCT(node) *conf, const double *grid)
{
	int	used = 0, adv, max, pos;
	
	if (len <= 0 || !ld)
		return -2;
	
	if ((max = ld) < 0) {
		max = -ld;
		ld  = 1;
		adv = len;
	} else {
		adv = 1;
	}
	
	do {
		MPT_INTERFACE(metatype) *meta;
		const char *descr;
		int	err = 0;
		
		if (!conf) break;
		
		if (!(descr = mpt_node_ident(conf))) {
			return used;
		}
		if (!(meta = conf->_meta)) {
			return used;
		}
		err = mpt_valtype_select(descr, (char **) &descr);
		
		switch (err) {
		  case 0: break; /* keep data */
		  case MPT_ENUM(ValueFormatLinear):
		  case MPT_ENUM(ValueFormatBoundaries):
		  case MPT_ENUM(ValueFormatText):
		  case MPT_ENUM(ValueFormatPolynom):
				if (!(descr = mpt_meta_data(meta, 0))) descr = "";
				if (mpt_valtype_init(len, dest, ld, descr, err, grid) < 0)
					return used;
				break;
			default:
				return used;
		}
		/* advance one profile */
		dest += adv;
		conf = conf->next;
		
		/* profile read */
	} while (++used < max);
	
	/* set remaining profiles to zero */
	if (!(pos = used)) {
		(void) mpt_values_bound(len, dest, ld, 0., 0., 0.);
		dest += adv;
		pos = 1;
	}
	/* fill remaining entries with last profile */
	while (pos++ < max) {
		mpt_copy64(len, dest-adv, ld, dest, ld);
		dest += adv;
	}
	
	return used;
}

