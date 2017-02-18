/*!
 * get logging descriptor in object.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "output.h"

#include "message.h"

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
	return mpt_output_vlog(w->out, from, type, fmt, arg);
}
static MPT_INTERFACE_VPTR(logger) logWrapperCtl = {
	{ logWrapperUnref },
	logWrapperLog
};

/*!
 * \ingroup mptCore
 * \brief log instance for output
 * 
 * Create log interface for output reference.
 * Returned logger instance takes ownership of passed reference.
 * 
 * \param out  output descriptor
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

