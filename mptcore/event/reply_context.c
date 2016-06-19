/*!
 * resolve and dispatch command.
 */

#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "event.h"

/*!
 * \ingroup mptEvent
 * \brief clear reply contexts
 * 
 * Clear pointer elements of used contexts and
 * delete unused elements.
 * 
 * \param base  context references base pointer
 * \param len   number of context references
 * 
 * \return created stream context
 */
extern size_t mpt_reply_clear(MPT_STRUCT(reply_context) **base, size_t len)
{
	size_t bad = 0;
	
	while (len) {
		MPT_STRUCT(reply_context) *ctx;
		if ((ctx = *base++)) {
			if (ctx->used) {
				++bad;
				ctx->ptr = 0;
			} else {
				base[-1] = 0;
				free(ctx);
			}
		}
	}
	return bad;
}

MPT_STRUCT(reply_context) *makeContext(size_t len)
{
	MPT_STRUCT(reply_context) *ctx;
	
	if (len < sizeof(ctx->_val)) {
		len = sizeof(ctx->_val);
	}
	if (!(ctx = malloc(sizeof(*ctx) - sizeof(ctx->_val) + len))) {
		return 0;
	}
	ctx->ptr = 0;
	ctx->_max = len;
	ctx->len = 0;
	ctx->used = 0;
	
	memset(ctx->_val, 0, len);
	
	return ctx;
}

/*!
 * \ingroup mptEvent
 * \brief event reply context
 * 
 * Create new or reuse available event reply context entry.
 * 
 * \param arr  array to register reply contexts on
 * \param len  needed reply context value size
 * 
 * \return available reply context
 */
extern MPT_STRUCT(reply_context) *mpt_reply_reserve(MPT_STRUCT(array) *arr, size_t len)
{
	MPT_STRUCT(reply_context) *ctx, **base;
	MPT_STRUCT(buffer) *buf;
	
	if ((buf = arr->_buf)) {
		size_t clen = buf->used / sizeof(*base);
		
		base = (void *) (buf + 1);
		while (clen--) {
			if ((ctx = *base++)) {
				if (!ctx->used && ctx->_max >= len) {
					return ctx;
				}
				continue;
			}
			if (!(ctx = makeContext(len))) {
				return 0;
			}
			base[-1] = ctx;
			return ctx;
		}
	}
	if (!(base = mpt_array_append(arr, sizeof(*base), 0))) {
		return 0;
	}
	if (!(ctx = makeContext(len))) {
		*base = 0;
		return 0;
	}
	return ctx;
}
