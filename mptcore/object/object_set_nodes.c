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
#include "output.h"

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
			val = mpt_object_set_property(obj, match, &conf->ident, (MPT_INTERFACE(convertable) *) conf->_meta);
			if (!val) {
				++proc;
			}
		}
		/* disable multiple object assignments */
		match &= ~MPT_ENUM(TraverseEmpty);
		if (!info) {
			continue;
		}
		if (val == MPT_ERROR(BadEncoding)) {
			mpt_log(info, __func__, MPT_LOG(Debug), "%s: %s: %02x",
				pr.name, MPT_tr("bad identifier type"), conf->ident._type);
			continue;
		}
		name = mpt_node_ident(conf);
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
				mpt_log(info, __func__, MPT_LOG(Warning), "%s: %s: %s",
				        pr.name, MPT_tr("bad property name"), name);
			} else {
				mpt_log(info, __func__, MPT_LOG(Warning), "%s: %s",
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
		if (val == MPT_ERROR(BadType)) {
			MPT_INTERFACE(metatype) *mt = conf->_meta;
			val = mt ? MPT_metatype_convert(mt, 0, 0) : 0;
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
			const char *type = conf->children ? "non-leaf" : "leaf";
			if (name) {
				mpt_log(info, __func__, MPT_LOG(Info), "%s.%s: %s (%s)",
				        pr.name, name, MPT_tr("skip node type"), MPT_tr(type));
			} else {
				mpt_log(info, __func__, MPT_LOG(Info), "%s: %s (%s)",
				        pr.name, MPT_tr("skip node type"), MPT_tr(type));
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

