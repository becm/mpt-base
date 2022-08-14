/*!
 * MPT plotting library
 *   configure history data
 */

#include <stdio.h>
#include <string.h>
#include <strings.h> /* for strcasecmp() */

#include "array.h"
#include "convert.h"
#include "types.h"

#include "object.h"

#include "values.h"

#include "history.h"

/*!
 * \ingroup mptHistory
 * \brief set history parameters
 * 
 * Change history settings.
 * 
 * \param hist  history descriptor
 * \param name  property to change
 * \param src   value source
 * 
 * \return consumed elements
 */
extern int mpt_history_set(MPT_STRUCT(history) *hist, const char *name, MPT_INTERFACE(convertable) *src)
{
	int ret;
	if (!name) {
		if ((ret = mpt_logfile_set(&hist->info, 0, src)) < 0) {
			return ret;
		}
		mpt_histfmt_reset(&hist->fmt);
		if (!src && !hist->info.file) {
			hist->info.file = stdout;
		}
		return ret;
	}
	if (!*name) {
		return MPT_ERROR(BadArgument);
	}
	/* accept various format assignment names */
	if (!strcasecmp(name, "format") || !strcasecmp(name, "histfmt") || !strcasecmp(name, "fmt")) {
		return mpt_valfmt_set(&hist->fmt._fmt, src);
	}
	/* remap file assignment name */
	if (!strcasecmp(name, "history") || !strcasecmp(name, "histfile") || !strcasecmp(name, "file")) {
		ret = mpt_logfile_set(&hist->info, "file", src);
		if (ret >= 0 && !src && !hist->info.file) {
			hist->info.file = stdout;
		}
		return ret;
	}
	return mpt_logfile_set(&hist->info, name, src);
}
/*!
 * \ingroup mptHistory
 * \brief get history parameters
 * 
 * Read history settings.
 * 
 * \param hist  history descriptor
 * \param pr    property to read
 * 
 * \return non-default length
 */
extern int mpt_history_get(const MPT_STRUCT(history) *hist, MPT_STRUCT(property) *pr)
{
	const char *name;
	intptr_t pos = -1, id;
	int len;
	
	if (!pr) {
		return MPT_ENUM(TypeFilePtr);
	}
	if (!(name = pr->name)) {
		pos = (intptr_t) pr->desc;
	}
	else if (!*name) {
		return mpt_logfile_get(&hist->info, pr);
	}
	id = 0;
	if (name ? (!strcasecmp(name, "format") || !strcasecmp(name, "histfmt") || !strcasecmp(name, "fmt")) :  pos == id++) {
		MPT_STRUCT(buffer) *buf;
		
		pr->name = "format";
		pr->desc = MPT_tr("history data output format");
		if (!(buf = hist->fmt._fmt._buf)
		    || !(len = buf->_used / sizeof(MPT_STRUCT(value_format)))) {
			MPT_property_set_string(pr, 0);
			return 0;
		}
		MPT_value_set(&pr->val, MPT_ENUM(TypeBufferPtr), &hist->fmt._fmt);
		return len;
	}
	else {
		MPT_STRUCT(property) pc = MPT_PROPERTY_INIT;
		if (!(pc.name = name)) {
			pc.desc = (char *) (id - 1);
		}
		else if (!strcasecmp(name, "history") || !strcasecmp(name, "histfile")) {
			pc.name = "file";
		}
		if ((len = mpt_logfile_get(&hist->info, &pc)) >= 0) {
			int ret;
			/* require plain copy of property buffer */
			if (pc.val.ptr == pc._buf) {
				if ((ret = mpt_value_copy(&pc.val, pr->_buf, sizeof(pr->_buf))) < 0) {
					return ret;
				}
				MPT_value_set(&pc.val, pc.val.type, pr->_buf);
			}
			else {
				MPT_value_set(&pr->val, pc.val.type, pc.val.ptr);
			}
			pr->name = pc.name;
			pr->desc = pc.desc;
		}
	}
	return len;
}
