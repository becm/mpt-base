/*!
 * resolve and dispatch command.
 */

#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "event.h"

MPT_STRUCT(deferrable_reply_context)
{
	int (*set)(void *, const MPT_STRUCT(message) *);
	
	MPT_STRUCT(refcount) ref;
	
	MPT_INTERFACE(reply_context) _ctx;
	MPT_STRUCT(reply_data) data;
};

static void contextUnref(MPT_INTERFACE(unrefable) *ref)
{
	MPT_STRUCT(deferrable_reply_context) *ctx = MPT_reladdr(deferrable_reply_context, ref, _ctx, set);
	
	if (mpt_refcount_lower(&ctx->ref)) {
		return;
	}
	if (ctx->data.len) {
		ctx->set(ctx->data.ptr, 0);
	}
	free(ctx);
}
static int contextSet(MPT_STRUCT(reply_context) *ptr, const MPT_STRUCT(message) *msg)
{
	MPT_STRUCT(deferrable_reply_context) *ctx = MPT_reladdr(deferrable_reply_context, ptr, _ctx, set);
	int ret;
	
	if (!ctx->set) {
		return 0;
	}
	ret = ctx->set(ctx->data.ptr, msg);
	if (ret >= 0) {
		ctx->data.len = 0;
	}
	return ret;
}
static MPT_STRUCT(reply_context) *contextDefer(MPT_STRUCT(reply_context) *ptr)
{
	MPT_STRUCT(deferrable_reply_context) *ctx = MPT_reladdr(deferrable_reply_context, ptr, _ctx, set);
	
	(void) ctx;
	/* TODO: implement deferrer instance
	if (!mpt_reference_raise(&ctx->ref)) {
		return 0;
	}*/
	return 0;
}
static const MPT_INTERFACE_VPTR(reply_context) _context_vptr = {
	{ contextUnref }, contextSet, contextDefer
};
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
extern MPT_INTERFACE(reply_context) *mpt_reply_deferrable(size_t len, int (*set)(void *, const MPT_STRUCT(message) *), void *arg)
{
	MPT_STRUCT(deferrable_reply_context) *ctx;
	
	if (len > UINT16_MAX) {
		return 0;
	}
	if (!(ctx = malloc(sizeof(*ctx) + len - sizeof(ctx->data.val)))) {
		return 0;
	}
	ctx->set = set;
	
	ctx->_ctx._vptr = &_context_vptr;
	
	ctx->data.ptr = arg;
	ctx->data._max = len;
	ctx->data.len = 0;
	
	return &ctx->_ctx;
}
