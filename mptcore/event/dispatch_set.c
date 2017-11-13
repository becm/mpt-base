/*!
 * register command in controller.
 */

#include "array.h"
#include "event.h"

/*!
 * \ingroup mptEvent
 * \brief register command
 * 
 * Set command with matching id or add new one.
 * Command id of zero (un)sets default command.
 * 
 * \param disp dispatch descriptor
 * \param id   id for command
 * \param cmd  command to set
 * \param arg  context for command
 * 
 * \return command (de)registration result
 */
extern int mpt_dispatch_set(MPT_STRUCT(dispatch) *disp, uintptr_t id, MPT_TYPE(EventHandler) cmd, void *arg)
{
	MPT_STRUCT(command) *dst = mpt_command_get(&disp->_d, id);
	
	/* clear registration */
	if (!cmd) {
		int pos;
		if (!dst) {
			return MPT_ERROR(BadArgument);
		}
		pos = dst - ((MPT_STRUCT(command) *) (disp->_d._buf + 1));
		dst->cmd(dst->arg, 0);
		dst->cmd = 0;
		dst->arg = 0;
		return pos;
	}
	/* id already used */
	if (dst) {
		return MPT_ERROR(BadArgument);
	}
	/* register command */
	else {
		MPT_STRUCT(command) reg;
		
		reg.id  = id;
		reg.cmd = (int (*)()) cmd;
		reg.arg = arg;
		
		return mpt_command_set(&disp->_d, &reg);
	}
}
