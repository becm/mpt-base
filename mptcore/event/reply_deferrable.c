/*!
 * resolve and dispatch command.
 */

#include <stdlib.h>
#include <string.h>

#include "meta.h"
#include "output.h"
#include "message.h"

#include "event.h"

MPT_STRUCT(reply_context_defer)
{
	struct {
		int (*send)(void *, const MPT_STRUCT(reply_data) *, const MPT_STRUCT(message) *);
		void *ptr;
	} reply;
	MPT_STRUCT(refcount)     ref;
	
	MPT_INTERFACE(metatype) _mt;
	MPT_INTERFACE(reply_context) _ctx;
	
	MPT_STRUCT(reply_data) data;
};
struct replyDataDelayed
{
	MPT_INTERFACE(reply_context_detached) _rc;
	MPT_STRUCT(reply_context_defer) *base;
	MPT_STRUCT(reply_data) data;
};

static int contextSend(const MPT_STRUCT(reply_context_defer) *ctx, MPT_STRUCT(reply_data) *rd, const MPT_STRUCT(message) *msg)
{
	int ret;
	uint64_t id = 0;
	
	if (!rd->len) {
		mpt_log(0, __func__, MPT_LOG(Warning), "%s: %s",
		        MPT_tr("bad reply operation"), MPT_tr("reply already sent"));
		return MPT_ERROR(BadArgument);
	}
	if (!ctx->reply.send) {
		rd->len = 0;
		return 0;
	}
	mpt_message_buf2id(rd->val, rd->len, &id);
	
	if (!ctx->reply.ptr) {
		mpt_log(0, __func__, MPT_LOG(Warning), "%s %s",
		        MPT_tr("unable to reply"), id, MPT_tr("no reply target available"));
		return 0;
	}
	rd->val[0] |= 0x80;
	ret = ctx->reply.send(ctx->reply.ptr, rd, msg);
	if (ret >= 0) {
		rd->len = 0;
	} else {
		rd->val[0] &= 0x7f;
		mpt_log(0, __func__, MPT_LOG(Error), "%s %s",
		        MPT_tr("unable to reply"), id, MPT_tr("reply send failed"));
	}
	return ret;
}
static void contextDetach(MPT_STRUCT(reply_context_defer) *ctx)
{
	if (mpt_refcount_lower(&ctx->ref)) {
		return;
	}
	free(ctx);
}
/* reference interface */
static void contextUnref(MPT_INTERFACE(metatype) *mt)
{
	MPT_STRUCT(reply_context_defer) *ctx = MPT_baseaddr(reply_context_defer, mt, _mt);
	
	if (mpt_refcount_lower(&ctx->ref)) {
		ctx->reply.send = 0;
		return;
	}
	if (ctx->reply.send && ctx->data.len) {
		contextSend(ctx, &ctx->data, 0);
	}
	free(ctx);
}
/* reference interface */
static uintptr_t contextRef(MPT_INTERFACE(metatype) *mt)
{
	MPT_STRUCT(reply_context_defer) *ctx = MPT_baseaddr(reply_context_defer, mt, _mt);
	return mpt_refcount_raise(&ctx->ref);
}
/* metatype interface */
static int contextConv(MPT_INTERFACE(convertable) *val, int type, void *ptr)
{
	MPT_STRUCT(reply_context_defer) *ctx = MPT_baseaddr(reply_context_defer, val, _mt);
	
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeReply), MPT_ENUM(TypeReplyData), 0 };
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
			return 0;
		}
		return MPT_ENUM(TypeReply);
	}
	if (type == MPT_ENUM(TypeReply)) {
		if (ptr) *((void **) ptr) = &ctx->_ctx;
		return MPT_ENUM(TypeReplyData);
	}
	if (type == MPT_ENUM(TypeReplyData)) {
		if (ptr) *((void **) ptr) = &ctx->_ctx;
		return MPT_ENUM(TypeReply);
	}
	return MPT_ERROR(BadType);
}
static MPT_INTERFACE(metatype) *contextClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* deferred context interface */
static int deferReply(MPT_STRUCT(reply_context_detached) *def, const MPT_STRUCT(message) *msg)
{
	struct replyDataDelayed *rd = (void *) def;
	int ret;
	if ((ret = contextSend(rd->base, &rd->data, msg)) < 0) {
		if (msg) {
			return ret;
		} else {
			ret = 0;
		}
	}
	contextDetach(rd->base);
	free(def);
	return ret;
}
/* reply context interface */
static int contextSet(MPT_STRUCT(reply_context) *ptr, const MPT_STRUCT(message) *msg)
{
	MPT_STRUCT(reply_context_defer) *ctx = MPT_baseaddr(reply_context_defer, ptr, _ctx);
	return contextSend(ctx, &ctx->data, msg);
}
static MPT_INTERFACE(reply_context_detached) *contextDefer(MPT_STRUCT(reply_context) *ptr)
{
	static const MPT_INTERFACE_VPTR(reply_context_detached) deferCtx = {
		deferReply
	};
	MPT_STRUCT(reply_context_defer) *ctx = MPT_baseaddr(reply_context_defer, ptr, _ctx);
	struct replyDataDelayed *def;
	size_t post;
	
	if (!ctx->data.len) {
		return 0;
	}
	if (!mpt_refcount_raise(&ctx->ref)) {
		return 0;
	}
	if ((post = ctx->data._max) < sizeof(ctx->data.val)) {
		post = 0;
	} else {
		post -= sizeof(ctx->data.val);
	}
	if (!(def = malloc(sizeof(*def) + post))) {
		mpt_refcount_lower(&ctx->ref);
		return 0;
	}
	def->_rc._vptr = &deferCtx;
	memcpy(&def->data, &ctx->data, sizeof(def->data) + post);
	def->base = ctx;
	
	/* reply data belongs to new context */
	ctx->data.len = 0;
	
	return &def->_rc;
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
extern MPT_INTERFACE(metatype) *mpt_reply_deferrable(size_t len, int (*send)(void *, const MPT_STRUCT(reply_data) *, const MPT_STRUCT(message) *), void *ptr)
{
	static const MPT_INTERFACE_VPTR(metatype) replyMeta = {
		{ contextConv },
		contextUnref,
		contextRef,
		contextClone
	};
	static const MPT_INTERFACE_VPTR(reply_context) replyCtx = {
		contextSet,
		contextDefer
	};
	MPT_STRUCT(reply_context_defer) *ctx;
	size_t post;
	
	if (len > UINT16_MAX) {
		return 0;
	}
	post = len > sizeof(ctx->data.val) ? len - sizeof(ctx->data.val) : 0;
	if (!(ctx = malloc(sizeof(*ctx) + post))) {
		return 0;
	}
	ctx->reply.send = send;
	ctx->reply.ptr  = ptr;
	
	ctx->ref._val = 1;
	
	ctx->_mt._vptr = &replyMeta;
	ctx->_ctx._vptr = &replyCtx;
	
	ctx->data._max = len;
	ctx->data.len = 0;
	
	return &ctx->_mt;
}
