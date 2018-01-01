/*!
 * set specific solver parameter from configuration list.
 */

#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>

#include "meta.h"
#include "node.h"
#include "object.h"

/*!
 * \ingroup mptSolver
 * \brief set solver properties
 * 
 * Set parameters from configuration elements.
 * 
 * \param gen   solver descriptor
 * \param match elements to process
 * \param conf  configuration list
 * \param out   logging descriptor
 * 
 * \return number of successfully processed elements
 */
extern int mpt_object_set_nodes(MPT_INTERFACE(object) *obj, int match, const MPT_STRUCT(node) *conf, MPT_INTERFACE(logger) *info)
{
	MPT_STRUCT(property) pr;
	int proc = 0;
	
	if (!conf) {
		return 0;
	}
	pr.name = "";
	pr.desc = 0;
	if (obj->_vptr->property(obj, &pr) < 0 || !pr.name) {
		pr.name = "object";
	}
	do {
		const char *name;
		int val;
		/* skip masked types */
		val = conf->children ? MPT_ENUM(TraverseNonLeafs) : MPT_ENUM(TraverseLeafs);
		
		if ((val & match)) {
			val = mpt_object_set_property(obj, match, &conf->ident, conf->_meta);
			if (!val) {
				++proc;
			}
		}
		/* disable multiple object assignments */
		match &= ~MPT_ENUM(TraverseEmpty);
		if (!info) {
			continue;
		}
		if (mpt_identifier_len(&conf->ident) > 0) {
			if (conf->ident._type
			    || !(name = mpt_identifier_data(&conf->ident))) {
				name = "";
			}
		} else {
			name = 0;
		}
		if (!val) {
			if (name) {
				mpt_log(info, __func__, MPT_LOG(Debug), "%s.%s: %s",
				        pr.name, name, MPT_tr("property modified"));
			} else {
				mpt_log(info, __func__, MPT_LOG(Debug), "%s: %s",
				        pr.name, MPT_tr("object modified"));
			}
			continue;
		}
		if (val == MPT_ERROR(BadArgument)) {
			if (name) {
				mpt_log(info, __func__, MPT_LOG(Error), "%s: %s: %s",
				        pr.name, MPT_tr("bad property name"), name);
			} else {
				mpt_log(info, __func__, MPT_LOG(Error), "%s: %s",
				        pr.name, MPT_tr("bad object assignment"));
			}
			continue;
		}
		if (val == MPT_ERROR(BadValue)) {
			if (name) {
				mpt_log(info, __func__, MPT_LOG(Error), "%s.%s: %s",
				        pr.name, name, MPT_tr("bad property value"));
			} else {
				mpt_log(info, __func__, MPT_LOG(Error), "%s: %s",
				        pr.name, MPT_tr("bad object assignment value"));
			}
			continue;
		}
		if (val == MPT_ERROR(BadValue)) {
			const MPT_INTERFACE(metatype) *mt = conf->_meta;
			val = mt ? mt->_vptr->conv(mt, 0, 0) : 0;
			if (name) {
				static const char *err = MPT_tr("bad property type");
				if (isalnum(val)) {
					mpt_log(info, __func__, MPT_LOG(Error), "%s.%s: %s: %c",
					        pr.name, name, err, val);
				} else {
					mpt_log(info, __func__, MPT_LOG(Error), "%s.%s: %s: 0x%x",
					        pr.name, name, err, val);
				}
			} else {
				static const char *err = MPT_tr("bad object assignment type");
				if (isalnum(val)) {
					mpt_log(info, __func__, MPT_LOG(Error), "%s: %s: %c",
					        pr.name, err, val);
				} else {
					mpt_log(info, __func__, MPT_LOG(Error), "%s.%s: %s: 0x%x",
					        pr.name, name, err, val);
				}
			}
			continue;
		}
		if (val < 0) {
			if (name) {
				mpt_log(info, __func__, MPT_LOG(Error), "%s.%s: %s",
				        pr.name, name, MPT_tr("bad property assignmnt"));
			} else {
				mpt_log(info, __func__, MPT_LOG(Error), "%s: %s",
				        pr.name, MPT_tr("bad object assignment"));
			}
			continue;
		}
		if (val & MPT_ENUM(TraverseEmpty)) {
			mpt_log(info, __func__, MPT_LOG(Info), "%s: %s",
			        pr.name, MPT_tr("skip object assignment"));
			continue;
		}
		if (val & MPT_ENUM(TraverseAll)) {
			if (name) {
				mpt_log(info, __func__, MPT_LOG(Debug), "%s.%s: %s",
				        pr.name, name, MPT_tr("skip node type"));
			} else {
				mpt_log(info, __func__, MPT_LOG(Debug), "%s: %s",
				        pr.name, MPT_tr("skip node type"));
			}
			continue;
		}
		if (val & MPT_ENUM(TraverseChange)) {
			if (name) {
				mpt_log(info, __func__, MPT_LOG(Debug), "%s.%s: %s",
				        pr.name, name, MPT_tr("skip property assignment"));
			} else {
				mpt_log(info, __func__, MPT_LOG(Debug), "%s: %s",
				        pr.name, MPT_tr("skip object assignment"));
			}
			continue;
		}
		if (val & MPT_ENUM(TraverseDefault)) {
			if (name) {
				mpt_log(info, __func__, MPT_LOG(Debug), "%s.%s: %s",
				        pr.name, name, MPT_tr("skip property reset"));
			} else {
				mpt_log(info, __func__, MPT_LOG(Debug), "%s: %s",
				        pr.name, MPT_tr("skip object reset"));
			}
			continue;
		}
	} while ((conf = conf->next));
	
	return proc;
}

