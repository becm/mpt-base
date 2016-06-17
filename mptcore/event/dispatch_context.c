/*!
 * resolve and dispatch command.
 */

#include <stdlib.h>

#include "array.h"
#include "message.h"

#include "event.h"

/*!
 * \ingroup mptEvent
 * \brief dispatch context
 * 
 * Create dispatch context for dispatch descriptor.
 * 
 * \param disp dispatch controller
 * 
 * \return existing or created dispatch context
 */
extern MPT_STRUCT(dispatch_context) *mpt_dispatch_context(MPT_STRUCT(dispatch_context) **disp)
{
	MPT_STRUCT(dispatch_context) *ctx = *disp;
	while (1) {
		/* no free context found */
		if (!ctx) {
			if (!(ctx = malloc(sizeof(*ctx)))) {
				return 0;
			}
			ctx->dsp = 0;
			ctx->_next = 0;
			*disp = ctx;
			return ctx;
		}
		/* reuse empty context */
		if (!ctx->dsp) {
			return ctx;
		}
		disp = &ctx->_next;
		ctx = ctx->_next;
	}
}
