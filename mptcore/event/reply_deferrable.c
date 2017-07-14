/*!
 * resolve and dispatch command.
 */

#include <stdlib.h>
#include <string.h>

#include "message.h"

#include "event.h"

MPT_STRUCT(reply_context_defer)
{
	int (*send)(const MPT_STRUCT(reply_data) *, const MPT_STRUCT(message) *);
	
	MPT_STRUCT(refcount) ref;
	
	MPT_INTERFACE(reply_context) _ctx;
	MPT_STRUCT(reply_data) data;
};

static int deferContextSet(const MPT_STRUCT(reply_context_defer) *ctx, MPT_STRUCT(reply_data) *rd, const MPT_STRUCT(message) *msg)
{
	int ret;
	uint64_t id = 0;
	void *save;
	
	if (!rd->len) {
		mpt_log(0, __func__, MPT_LOG(Warning), "%s: %s",
		        MPT_tr("bad reply operation"), MPT_tr("reply already sent"));
		return MPT_ERROR(BadArgument);
	}
	if (!ctx->send) {
		rd->len = 0;
		return 0;
	}
	mpt_message_buf2id(rd->val, rd->len, &id);
	
	if (!ctx->data.ptr) {
		mpt_log(0, __func__, MPT_LOG(Warning), "%s %s",
		        MPT_tr("unable to reply"), id, MPT_tr("no reply target available"));
		return 0;
	}
	rd->val[0] |= 0x80;
	save = rd->ptr;
	rd->ptr = ctx->data.ptr;
	ret = ctx->send(rd, msg);
	rd->ptr = save;
	rd->val[0] &= 0x7f;
	if (ret >= 0) {
		rd->len = 0;
	} else {
		mpt_log(0, __func__, MPT_LOG(Error), "%s %s",
		        MPT_tr("unable to reply"), id, MPT_tr("reply send failed"));
	}
	return ret;
}

static void contextUnref(MPT_INTERFACE(unrefable) *ref)
{
	MPT_STRUCT(reply_context_defer) *ctx = MPT_reladdr(reply_context_defer, ref, _ctx, send);
	
	if (mpt_refcount_lower(&ctx->ref)) {
		return;
	}
	if (ctx->send && ctx->data.len) {
		ctx->send(&ctx->data, 0);
	}
	free(ctx);
}
static int contextSet(MPT_STRUCT(reply_context) *ptr, const MPT_STRUCT(message) *msg)
{
	MPT_STRUCT(reply_context_defer) *ctx = MPT_reladdr(reply_context_defer, ptr, _ctx, send);
	return deferContextSet(ctx, &ctx->data, msg);
}


static void deferUnref(MPT_INTERFACE(unrefable) *ref)
{
	MPT_STRUCT(reply_data) *rd = (void *) (ref + 1);
	MPT_STRUCT(reply_context_defer) *ctx = rd->ptr;
	if (rd->len) {
		deferContextSet(ctx, rd, 0);
	}
	contextUnref((void *) &ctx->_ctx);
	free(ref);
}
static int deferSet(MPT_STRUCT(reply_context) *def, const MPT_STRUCT(message) *msg)
{
	MPT_STRUCT(reply_data) *rd = (void *) (def + 1);
	return deferContextSet(rd->ptr, rd, msg);
}
static MPT_STRUCT(reply_context) *deferContext(MPT_STRUCT(reply_context) *ptr)
{
	(void) ptr;
	return 0;
}
static MPT_STRUCT(reply_context) *contextDefer(MPT_STRUCT(reply_context) *ptr)
{
	static const MPT_INTERFACE_VPTR(reply_context) def_vptr = {
		{ deferUnref },
		deferSet,
		deferContext
	};
	MPT_STRUCT(reply_context_defer) *ctx = MPT_reladdr(reply_context_defer, ptr, _ctx, send);
	MPT_INTERFACE(reply_context) *def;
	MPT_STRUCT(reply_data) *rd;
	
	if (!ctx->data.len) {
		return 0;
	}
	if (!mpt_refcount_raise(&ctx->ref)) {
		return 0;
	}
	if (!(def = malloc(sizeof(*def) + sizeof(*rd) + ctx->data._max - sizeof(ctx->data.val)))) {
		mpt_refcount_lower(&ctx->ref);
		return 0;
	}
	def->_vptr = &def_vptr;
	rd = memcpy(def + 1, &ctx->data, sizeof(*rd) + ctx->data.len - sizeof(ctx->data.val));
	rd->ptr = ctx;
	
	/* reply data belongs to new context */
	ctx->data.len = 0;
	
	return def;
}
static const MPT_INTERFACE_VPTR(reply_context) _context_vptr = {
	{ contextUnref },
	contextSet,
	contextDefer
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
extern MPT_INTERFACE(reply_context) *mpt_reply_deferrable(size_t len, int (*send)(const MPT_STRUCT(reply_data) *, const MPT_STRUCT(message) *))
{
	MPT_STRUCT(reply_context_defer) *ctx;
	
	if (len > UINT16_MAX) {
		return 0;
	}
	if (!(ctx = malloc(sizeof(*ctx) + len - sizeof(ctx->data.val)))) {
		return 0;
	}
	ctx->send = send;
	ctx->ref._val = 1;
	
	ctx->_ctx._vptr = &_context_vptr;
	
	ctx->data.ptr = 0;
	ctx->data._max = len;
	ctx->data.len = 0;
	
	return &ctx->_ctx;
}
