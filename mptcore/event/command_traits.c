/*!
 * MPT core library
 *   type traits for command data
 */

#include <string.h>

#include "types.h"

#include "event.h"

static void _command_fini(void *ptr)
{
	MPT_STRUCT(command) *c = ptr;
	if (c->cmd) {
		c->cmd(c->arg, 0);
	}
}
static int _command_init(void *ptr, const void *src)
{
	const MPT_STRUCT(command) *c = src;
	if (c && c->cmd) {
		return MPT_ERROR(BadOperation);
	}
	memset(ptr, 0, sizeof(*c));
	return 0;
}

/*!
 * \ingroup mptEvent
 * \brief get command traits
 * 
 * Get command operations and size.
 * 
 * \return command type traits
 */
extern const MPT_STRUCT(type_traits) *mpt_command_traits()
{
	static const MPT_STRUCT(type_traits) traits = {
		_command_init,
		_command_fini,
		sizeof(MPT_STRUCT(command))
	};
	return &traits;
}
