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
 * Find/Create available dispatch context entry.
 * 
 * \param ptr  reference to context base pointer
 * 
 * \return existing or created dispatch context
 */
extern MPT_STRUCT(dispatch_context) *mpt_dispatch_context(MPT_STRUCT(dispatch_context) **ptr)
{
	MPT_STRUCT(dispatch_context) *ctx = *ptr;
	while (1) {
		/* no free context found */
		if (!ctx) {
			if (!(ctx = malloc(sizeof(*ctx)))) {
				return 0;
			}
			ctx->dsp = 0;
			ctx->_next = 0;
			*ptr = ctx;
			return ctx;
		}
		/* reuse empty context */
		if (!ctx->dsp) {
			return ctx;
		}
		ptr = &ctx->_next;
		ctx = ctx->_next;
	}
}
