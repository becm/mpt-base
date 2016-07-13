/*!
 * get logging descriptor in object.
 */

#include <stdlib.h>

#include "output.h"

struct mptLogWrapper
{
	MPT_INTERFACE(logger) _log;
	MPT_INTERFACE(output) *out;
};

static void logWrapperUnref(MPT_INTERFACE(unrefable) *l)
{
	struct mptLogWrapper *w = (void *) l;
	w->out->_vptr->obj.ref.unref((void *) w->out);
	free(w);
}
static int logWrapperLog(MPT_INTERFACE(logger) *l, const char *from, int type, const char *fmt, va_list arg)
{
	struct mptLogWrapper *w = (void *) l;
	return w->out->_vptr->log(w->out, from, type, fmt, arg);
}
static MPT_INTERFACE_VPTR(logger) logWrapperCtl = {
	{ logWrapperUnref },
	logWrapperLog
};

/*!
 * \ingroup mptCore
 * \brief log instance of object
 * 
 * Search log interface in object properties.
 * 
 * \param obj  object descriptor
 * 
 * \return logging descriptor
 */
extern MPT_INTERFACE(logger) *mpt_output_logger(MPT_INTERFACE(output) *out)
{
	struct mptLogWrapper *w;
	
	if (!out || !(w = malloc(sizeof(*w)))) {
		return 0;
	}
	w->_log._vptr = &logWrapperCtl;
	w->out = out;
	
	return &w->_log;
}

