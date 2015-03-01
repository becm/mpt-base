/*!
 * initialize/terminate event controller.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

#include "array.h"
#include "event.h"

/*!
 * \ingroup mptEvent
 * \brief register command
 * 
 * Set command with matching id or add new one.
 * 
 * \param arr  command control array
 * \param cmd  command to set
 * 
 * \return command registration result
 */
extern int mpt_command_set(MPT_STRUCT(array) *arr, const MPT_STRUCT(command) *cmd)
{
	MPT_STRUCT(command) *dest;
	
	/* replace/delete command */
	if ((dest = mpt_command_get(arr, cmd->id))) {
		dest->cmd(dest->arg, 0);
		if (!cmd->cmd) {
			dest->cmd = 0;
			return 0;
		}
		*dest = *cmd;
		return 2;
	}
	/* place in empty area */
	if (arr->_buf) {
		size_t	len;
		
		dest = (void *) (arr->_buf+1);
		len = arr->_buf->used/sizeof(*dest);
		
		if (len && (dest = mpt_command_empty(dest, len))) {
			*dest = *cmd;
			return 0;
		}
	}
	/* append command data */
	if (!(dest = mpt_array_append(arr, sizeof(*cmd), cmd)))
		return -1;
	
	return 1;
}
