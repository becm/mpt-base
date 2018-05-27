/*!
 * MPT core library
 *   unique command creation
 */

#include <inttypes.h>
#include <string.h>

#include "message.h"
#include "output.h"

#include "event.h"

static int logReply(void *out, void *arg)
{
	static const char _func[] = "mpt::command::reply\0";
	MPT_STRUCT(message) msg;
	MPT_STRUCT(msgtype) mt;
	int len;
	
	if (!arg) {
		mpt_log(0, _func, MPT_LOG(Debug), "%s (" PRIxPTR ")",
		        MPT_tr("empty reply"), out);
		return 0;
	}
	memcpy(&msg, arg, sizeof(msg));
	if (!(len = mpt_message_read(&msg, sizeof(mt), &mt))) {
		mpt_log(0, _func, MPT_LOG(Debug), "%s (" PRIxPTR ")",
		        MPT_tr("zero length reply"), out);
		return 0;
	}
	if (mt.cmd == MPT_MESGTYPE(Answer)) {
		int type = MPT_LOG(Debug);
		if (len < 2) {
			mt.arg = 0;
		} else if (mt.arg < 0) {
			type = MPT_LOG(Error);
		} else if (mt.arg) {
			type = MPT_LOG(Info);
		}
		mpt_log(0, _func, type, "%s (" PRIxPTR "): %s = %02x",
		        MPT_tr("answer message"), out, MPT_tr("code"), mt.arg);
	}
	else if (mt.cmd == MPT_MESGTYPE(Output)) {
		if (len < 2) {
			mt.arg = MPT_LOG(Debug);
		}
		mpt_log(0, _func, mt.arg & 0x7f, "%s (" PRIxPTR ")",
		        MPT_tr("reply message"), out);
	}
	else {
		len += mpt_message_length(&msg);
		mpt_log(0, _func, MPT_LOG(Debug), "%s (" PRIxPTR "): %s = %02x, %s = %i",
		        MPT_tr("message"), out,
		        MPT_tr("type"), mt.cmd,
		        MPT_tr("length"), len);
	}
	return 0;
}
/*!
 * \ingroup mptEvent
 * \brief create unique id command
 * 
 * Add new command with unique identifier in message range.
 * Set control handler of returned element to activate.
 * 
 * \param arr  command control array
 * 
 * \return registered command
 */
extern MPT_STRUCT(command) *mpt_command_nextid(MPT_STRUCT(array) *arr, size_t max)
{
	MPT_STRUCT(buffer) *msg;
	MPT_STRUCT(command) *base, *cmd = 0;
	size_t i, len, used = 0;
	uintptr_t mid = 0;
	
	
	switch (max) {
	  case 0: return 0;
	  case 1: max = INT8_MAX; break;
	  case 2: max = INT16_MAX; break;
	  case 3: max = INT32_MAX/0x100; break;
	  case 4: max = INT32_MAX; break;
	  case 5: max = INT64_MAX/0x1000000; break;
	  case 6: max = INT64_MAX/0x10000; break;
	  case 7: max = INT64_MAX/0x100; break;
	  default: max = INT64_MAX;
	}
	if (max > INTPTR_MAX) {
		max = INTPTR_MAX;
	}
	/* command data on buffer */
	if ((msg = arr->_buf)) {
		len  = msg->_used / sizeof(*cmd);
		base = (void *) (msg+1);
	} else {
		if ((cmd = mpt_array_append(arr, sizeof(*cmd) * 8, 0))) {
			static const uintptr_t firstId = 1;
			cmd->id  = firstId;
			cmd->cmd = logReply;
			cmd->arg = (void *) firstId;
		}
		return cmd;
	}
	for (i = 0; i < len; ++i) {
		/* find highest previous id */
		if (base[i].id > mid) {
			mid = base[i].id;
		}
		/* save available space */
		if (!base[i].cmd) {
			if (!cmd) {
				cmd = base+i;
			}
			continue;
		}
		/* track number of active commands */
		++used;
		
		/* no smaller position available */
		if (!cmd) continue;
		
		/* move command entry */
		*cmd = base[i];
		base[i].cmd = 0;
		
		/* find smallest free position */
		while (++cmd < (base+i)) {
			if (!cmd->cmd) {
				break;
			}
		}
	}
	/* save used size */
	msg->_used = used * sizeof(*cmd);
	
	/* try to find low free id */
	if (++mid > max) {
		for (i = 1; i <= max; ++i) {
			if (!mpt_command_find(base, used, i)) {
				mid = i;
				break;
			}
		}
		/* no unique message id available */
		if (i > max) {
			return 0;
		}
	}
	/* add command slot */
	if (!(cmd = mpt_array_append(arr, sizeof(*cmd), 0))) {
		return 0;
	}
	
	/* setup default handling */
	cmd->id  = mid;
	cmd->cmd = logReply;
	cmd->arg = (void *) mid;
	
	return cmd;
}
