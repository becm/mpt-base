/*!
 * append profile data to buffer
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "node.h"
#include "array.h"

#include "values.h"

/*!
 * \ingroup mptValues
 * \brief configure parameter data
 * 
 * Get profile data from configuration element.
 * 
 * \param arr	target array descriptor
 * \param len	length of profiles
 * \param neqs	number of profiles
 * \param conf	profile configuration node
 * \param out	error log descriptor
 * 
 * \return zero on success
 */
extern int mpt_conf_profile(const MPT_STRUCT(array) *arr, int len, int neqs, const MPT_STRUCT(node) *conf, MPT_INTERFACE(logger) *out)
{
	const MPT_STRUCT(node) *prof;
	MPT_STRUCT(buffer) *buf;
	const char *desc;
	double	*val;
	int	ld = 1, err;

	if (len < 1 || neqs < 1) {
		(void) mpt_log(out, __func__, MPT_ENUM(LogError), "%s: (len = %i) > (neqs = %i)", MPT_tr("bad solver data size"), len, neqs);
		return -1;
	}
	ld  = (buf = arr->_buf) ? ((buf->used / sizeof(double)) / len) : 0;
	val = (double *) (buf+1);
	
	if (neqs > --ld) {
		(void) mpt_log(out, __func__, MPT_ENUM(LogError), "%s: (npde = %i) > (ld = %i)", MPT_tr("bad user data allocation"), neqs, ld);
		return -1;
	}
	
	if (!(prof = conf) || (prof = conf->children) || !(desc = mpt_node_data(conf, 0))) {
		int	np;
		
		if ((np = mpt_conf_profiles(len, val+len, neqs, prof, val)) < 0) {
			(void) mpt_log(out, __func__, MPT_ENUM(LogError), "%s", MPT_tr("invalid start values"));
			return -2;
		}
		else if (!np) {
			(void) mpt_log(out, __func__, MPT_ENUM(LogError), "%s", MPT_tr("use zero initial values"));
		} else if (np < neqs) {
			(void) mpt_log(out, __func__, MPT_ENUM(LogError), "%s (%d..%d)", MPT_tr("use latest initial values"), np, neqs);
		}
	}
	else if (neqs > 1) {
		(void) mpt_log(out, __func__, MPT_ENUM(LogError), "%s", MPT_tr("PDE needs verbose profile configuration"));
		return -2;
	}
	else if ((err = mpt_valtype_select(desc, (char **) &desc)) < 0) {
		(void) mpt_log(out, __func__, MPT_ENUM(LogError), "%s: %s", MPT_tr("bad profile description"), desc);
		return -2;
	}
	else if (mpt_valtype_init(len, val+len, neqs, desc, err, val) < 0) {
		(void) mpt_log(out, __func__, MPT_ENUM(LogError), "%s: %s", MPT_tr("bad profile parameter"), desc);
		return -2;
	}
	/* set used space */
	buf->used = (neqs+1) * len * sizeof(double);
	
	return 0;
}

