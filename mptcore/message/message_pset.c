/*!
 * argument to property conversion
 */

#include <errno.h>

#include <string.h>

#include "config.h"
#include "message.h"

/*!
 * \ingroup mptMessage
 * \brief set properties from arguments
 * 
 * Emit properties converted from message arguments.
 * 
 * \param msg	message data
 * \param type	argument separator
 * \param sfcn	property handler function
 * \param spar	handler function argument
 * 
 * \retval -2	no further arguments
 * \retval -1	property setting failed
 * \retval >=0	property set result
 */
extern int mpt_message_pset(MPT_STRUCT(message) *msg, int type, MPT_TYPE(PropertyHandler) sfcn, void *spar)
{
	MPT_STRUCT(message) tmp;
	MPT_STRUCT(property) prop;
	char buf[1024], *sep;
	ssize_t part, len;
	
	tmp = *msg;
	if ((part = mpt_message_argv(&tmp, type)) <= 0) {
		return -2;
	}
	if (part >= (ssize_t) sizeof(buf)) {
		return -1;
	}
	mpt_message_read(&tmp, part, buf);
	buf[part] = '\0';
	
	if (!(sep = memchr(buf, '=', part))) {
		return -3;
	}
	*sep = 0;
	
	prop.name = buf;
	prop.desc = 0;
	prop.val.fmt = 0;
	prop.val.ptr = ++sep;
	
	len = part - (sep - buf);
	
	if (*buf == '\'' || *buf == '"') {
		if (*buf == sep[len-1]) {
			sep[len-1] = '\0';
			++prop.name;
		}
	}
	else if (*sep == '\'' || *buf == '"') {
		if (*sep == sep[len-1]) {
			sep[len-1] = '\0';
			prop.val.ptr = ++sep;
		}
	}
	*msg = tmp;
	if ((part = sfcn(spar, &prop)) >= 0) {
		return part ? part : 1;
	}
	return 0;
}
