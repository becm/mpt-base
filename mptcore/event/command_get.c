/*!
 * command buffer search/delete operations.
 */

#include <string.h>
#include <errno.h>

#include "array.h"
#include "event.h"

/*!
 * \ingroup mptEvent
 * \brief clear commands
 * 
 * Remove all registered commands from dispatcher.
 * 
 * \param arr  command array
 */
extern void mpt_command_clear(const MPT_STRUCT(array) *arr)
{
	MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(command) *cmd;
	size_t i, len;
	
	if (!(buf = arr->_buf)) {
		return;
	}
	len = buf->_used / sizeof(*cmd);
	cmd = (void *) (buf + 1);
	
	for (i = 0; i < len; ++i) {
		if (cmd[i].cmd) {
			cmd[i].cmd(cmd[i].arg, 0);
		}
	}
	buf->_used = 0;
}
/*!
 * \ingroup mptEvent
 * \brief get empty command
 * 
 * Get pointer to command without control handler
 * indicating unused element.
 * 
 * \param cmd  command array base address
 * \param elem number of consecutive command elements
 * 
 * \return pointer to first unused element
 */
extern MPT_STRUCT(command) *mpt_command_empty(const MPT_STRUCT(command) *cmd, size_t elem)
{
	while (elem--) {
		if (!cmd->cmd) {
			return (MPT_STRUCT(command) *) cmd;
		}
		cmd++;
	}
	return 0;
}

/*!
 * \ingroup mptEvent
 * \brief get active command
 * 
 * Get pointer to command with control handler and matching id.
 * 
 * \param cmd  command array base address
 * \param elem number of consecutive command elements
 * \param id   identifier of requested command
 * 
 * \return pointer to matching element
 */
extern MPT_STRUCT(command) *mpt_command_find(const MPT_STRUCT(command) *cmd, size_t elem, uintptr_t id)
{
	while (elem--) {
		if (cmd->cmd && id == cmd->id) {
			return (MPT_STRUCT(command) *) cmd;
		}
		cmd++;
	}
	return 0;
}

/*!
 * \ingroup mptEvent
 * \brief get active command
 * 
 * Get pointer to command with control handler and matching id.
 * 
 * \param arr  array containgin command elements
 * \param id   identifier of requested command
 * 
 * \return pointer to matching element
 */
extern MPT_STRUCT(command) *mpt_command_get(const MPT_STRUCT(array) *arr, uintptr_t id)
{
	MPT_STRUCT(buffer) *buf = arr->_buf;
	size_t len;
	
	if (!buf || !(len = buf->_used / sizeof(MPT_STRUCT(command)))) {
		return 0;
	}
	return mpt_command_find((void *) (buf + 1), len, id);
}
