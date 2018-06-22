/*!
 * client metatype registration.
 */

#include <string.h>
#include <inttypes.h>

#include "array.h"
#include "message.h"

#include "client.h"

/*!
 * \ingroup mptClient
 * \brief client command
 * 
 * Process command message with client.
 * 
 * \return id for client metatype
 */
extern int mpt_client_command(MPT_INTERFACE(client) *cl, const MPT_STRUCT(message) *msg, int sep)
{
	MPT_STRUCT(array) a = MPT_ARRAY_INIT;
	MPT_INTERFACE(metatype) *arg;
	MPT_INTERFACE(iterator) *it;
	const char *cmd;
	uintptr_t id;
	int ret;
	
	if ((ret = mpt_array_message(&a, msg, sep)) < 0) {
		return ret;
	}
	arg = mpt_meta_arguments(&a);
	mpt_array_clone(&a, 0);
	
	if (!arg) {
		return MPT_ERROR(BadOperation);
	}
	cmd = 0;
	id = 0;
	if ((ret = arg->_vptr->conv(arg, 's', &cmd)) >= 0
	    && cmd) {
		id = mpt_hash(cmd, strlen(cmd));
	}
	it = 0;
	ret = arg->_vptr->conv(arg, MPT_type_pointer(MPT_ENUM(TypeIterator)), &it);
	
	ret = cl->_vptr->process(cl, id, it);
	arg->_vptr->ref.unref((void *) arg);
	
	return ret;
}
