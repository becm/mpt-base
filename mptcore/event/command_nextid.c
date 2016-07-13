/*!
 * MPT core library
 *   unique command creation
 */

#include <stdio.h>
#include <string.h>
#include <sys/uio.h>

#include "event.h"
#include "array.h"
#include "message.h"

static int printReply(void *out, void *msg)
{
	if (!out) {
		return 0;
	}
	fputs(" >", out);
	return mpt_message_print(out, msg);
}
/*!
 * \ingroup mptEvent
 * \brief create unique id command
 * 
 * Add new command with unique identifier in message range.
 * Set control handler of returned element to activate.
 * 
 * \param arr	command control array
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
		len  = msg->used / sizeof(*cmd);
		base = (void *) (msg+1);
	} else {
		if ((cmd = mpt_array_append(arr, sizeof(*cmd), 0))) {
			cmd->id  = 1;
			cmd->cmd = (int (*)()) mpt_message_print;
			cmd->arg = stderr;
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
	msg->used = used * sizeof(*cmd);
	
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
	cmd->cmd = printReply;
	cmd->arg = stderr;
	
	return cmd;
}
