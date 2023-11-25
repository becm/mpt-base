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
 * \param id   identifer for command
 * \param cmd  command to set
 * \param arg  context for command
 * 
 * \return command registration result
 */
extern int mpt_command_set(_MPT_UARRAY_TYPE(command) *arr, uintptr_t id, int (*cmd)(void *, void *), void *arg)
{
	MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(command) *dest;
	
	/* replace/delete command */
	if ((dest = mpt_command_get(arr, id))) {
		dest->cmd(dest->arg, 0);
		dest->cmd = cmd;
		dest->arg = arg;
		return cmd ? 0 : 2;
	}
	if (!(buf = arr->_buf)) {
		/* reject copy (command has singular context reference!) */
		if (!(buf = _mpt_buffer_alloc(sizeof(*dest), MPT_ENUM(BufferNoCopy)))) {
			return MPT_ERROR(BadOperation);
		}
		buf->_content_traits = mpt_command_traits();
		arr->_buf = buf;
		if (!(dest = mpt_buffer_insert(buf, 0, sizeof(*dest)))) {
			return MPT_ERROR(BadOperation);
		}
	}
	else {
		size_t len = buf->_used / sizeof(*dest);
		
		/* place in empty area */
		if (len && (dest = mpt_command_empty((void *) (buf + 1), len))) {
			dest->id = id;
			dest->cmd = cmd;
			dest->arg = arg;
			return 0;
		}
		/* append command data */
		if (!(dest = mpt_array_insert(arr, buf->_used, sizeof(*dest)))) {
			return MPT_ERROR(BadOperation);
		}
	}
	dest->id = id;
	dest->cmd = cmd;
	dest->arg = arg;
	
	return 1;
}
