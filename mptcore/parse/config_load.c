/*!
 * resolve alias to full library description
 */

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "node.h"
#include "config.h"

#include "parse.h"

struct loadCtx
{
	MPT_INTERFACE(logger) *log;
	MPT_STRUCT(node) *root;
	const char *func;
};

static int nodeGlobal(void *ptr, const MPT_STRUCT(path) *p, int last, int curr)
{
	
	struct loadCtx *ctx = ptr;
	
	(void) last;
	
	if ((curr & 0x3) != MPT_PARSEFLAG(Option)) {
		return 0;
	}
	if (!(mpt_node_assign(&ctx->root, p))) {
		if (ctx->log) {
			mpt_log(ctx->log, ctx->func, MPT_LOG(Error), "%s",
			        MPT_tr("failed to set global config element"));
		}
		return MPT_ERROR(BadOperation);
	}
	return 0;
}
static int nodeSet(void *ptr, const MPT_STRUCT(path) *p, int last, int curr)
{
	
	struct loadCtx *ctx = ptr;
	
	(void) last;
	
	if ((curr & 0x3) != MPT_PARSEFLAG(Option)) {
		return 0;
	}
	if (!(mpt_node_assign(&ctx->root->children, p))) {
		if (ctx->log) {
			mpt_log(ctx->log, ctx->func, MPT_LOG(Error), "%s",
			        MPT_tr("failed to set global config element"));
		}
		return MPT_ERROR(BadOperation);
	}
	return 0;
}
/*!
 * \ingroup mptParse
 * \brief read global mpt config
 * 
 * Process configuration file/directory.
 * 
 * \param root  configuration in alternative file root
 * \param log   optional log descriptor
 * \param dest  target config path
 * 
 * \return configuration load result
 */
extern int mpt_config_load(const char *root, MPT_INTERFACE(logger) *log, const MPT_STRUCT(path) *dest)
{
	struct loadCtx ctx;
	
	if (dest && !dest->len) {
		ctx.root = mpt_config_node(0);
	} else {
		ctx.root = mpt_config_node(dest);
	}
	if (!ctx.root) {
		if (log) {
			mpt_log(log, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("require existing global config target"));
		}
		return MPT_ERROR(BadArgument);
	}
	ctx.func = __func__;
	if (dest && !dest->len) {
		return _mpt_config_load(root, ctx.log = log, nodeGlobal, &ctx);
	}
	return _mpt_config_load(root, ctx.log = log, nodeSet, &ctx);
}
