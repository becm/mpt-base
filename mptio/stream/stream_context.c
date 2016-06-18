/*!
 * resolve and dispatch command.
 */

#include <stdlib.h>

#include "array.h"
#include "message.h"

#include "stream.h"

/*!
 * \ingroup mptStream
 * \brief stream context
 * 
 * Find/Create available stream context entry.
 * 
 * \param ptr  reference to context base pointer
 * 
 * \return existing or created stream context
 */
extern MPT_STRUCT(stream_context) *mpt_stream_context(MPT_STRUCT(stream_context) **ptr, size_t len)
{
	MPT_STRUCT(stream_context) *ctx = *ptr;
	while (1) {
		/* no free context found */
		if (!ctx) {
			if (!(ctx = malloc(sizeof(*ctx) + len))) {
				return 0;
			}
			ctx->srm = 0;
			ctx->_next = 0;
			ctx->len = len;
			*ptr = ctx;
			return ctx;
		}
		/* reuse empty context */
		if (!ctx->srm && ctx->len <= len) {
			return ctx;
		}
		ptr = &ctx->_next;
		ctx = ctx->_next;
	}
}
