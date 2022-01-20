/*!
 * argument to property conversion
 */

#include <errno.h>

#include <string.h>

#include "message.h"

#include "object.h"

/*!
 * \ingroup mptMessage
 * \brief set properties from arguments
 * 
 * Emit properties converted from message arguments.
 * 
 * \param msg   message data
 * \param sep   argument separator
 * \param sfcn  property handler function
 * \param spar  handler function argument
 * 
 * \retval mpt::MissingData    no further arguments
 * \retval mpt::MissingBuffer  message content too large
 * \retval mpt::BadEncoding    missing property name separator
 * \retval mpt::BadValue       property processing failed
 * \retval >=0                 property set result
 */
extern int mpt_message_property(MPT_STRUCT(message) *msg, int sep, MPT_TYPE(property_handler) sfcn, void *spar)
{
	MPT_STRUCT(message) tmp;
	MPT_STRUCT(property) prop = MPT_PROPERTY_INIT;
	char buf[1024], *pos;
	ssize_t part, len;
	
	tmp = *msg;
	if ((part = mpt_message_argv(&tmp, sep)) <= 0) {
		return MPT_ERROR(MissingData);
	}
	if (part >= (ssize_t) sizeof(buf)) {
		return MPT_ERROR(MissingBuffer);
	}
	mpt_message_read(&tmp, part, buf);
	buf[part] = '\0';
	
	if (!(pos = memchr(buf, '=', part))) {
		return MPT_ERROR(BadEncoding);
	}
	*pos = 0;
	
	prop.name = buf;
	MPT_value_set_string(&prop.val, ++pos);
	
	len = part - (pos - buf);
	
	if (*buf == '\'' || *buf == '"') {
		if (*buf == pos[len - 1]) {
			pos[len - 1] = '\0';
			++prop.name;
		}
	}
	else if (*pos == '\'' || *pos == '"') {
		if (*pos == pos[len - 1]) {
			pos[len - 1] = '\0';
			MPT_value_set_string(&prop.val, ++pos);
		}
	}
	if ((part = sfcn(spar, &prop)) < 0) {
		return MPT_ERROR(BadValue);
	}
	*msg = tmp;
	return part;
}
