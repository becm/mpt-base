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
	MPT_STRUCT(command) *dst = mpt_command_get(&disp->_cmd, id);
	
	/* clear registration */
	if (!cmd) {
		if (!dst) {
			return -2;
		}
		return dst - ((MPT_STRUCT(command) *) (disp->_cmd._buf + 1));
	}
	/* id already used */
	if (dst) {
		return -2;
	}
	/* register command */
	else {
		MPT_STRUCT(command) reg;
		
		reg.id  = id;
		reg.cmd = (int (*)()) cmd;
		reg.arg = arg;
		
		return mpt_command_set(&disp->_cmd, &reg);
	}
}
