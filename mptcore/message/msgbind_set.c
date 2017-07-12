/*!
 * get binding source element
 */

#include <ctype.h>

#include "convert.h"

#include "message.h"

/*!
 * \ingroup mptMessage
 * \brief set binding source
 * 
 * Parse single binding source parameter.
 * Call in loop to get all sources for description.
 * 
 * \param[in,out] bnd  binding source
 * \param         data text describing binding sources
 * 
 * \return length of consumed text
 */
extern int mpt_msgbind_set(MPT_STRUCT(msgbind) *bnd, const char *data)
{
	uint8_t val, state;
	int curr;
	
	/* special validity for binding */
	switch (data[0]) {
		case 'i': state = MPT_DATASTATE(Init); break;
		case 'I': state = MPT_DATASTATE(All) & ~MPT_DATASTATE(Init); break;
		case 's': state = MPT_DATASTATE(Step); break;
		case 'S': state = MPT_DATASTATE(All) & ~MPT_DATASTATE(Step); break;
		case 'f': state = MPT_DATASTATE(Fini); break;
		case 'F': state = MPT_DATASTATE(All) & ~MPT_DATASTATE(Fini); break;
		case 'A': state = 0; break;
		case 'a':
		default:  state = MPT_DATASTATE(All);
	}
	if (isalpha(data[0])) {
		++data;
	}
	if ((curr = mpt_cuint8(&val, data, 0, 0)) <= 0) {
		return curr;
	}
	bnd->dim  = val;
	bnd->state = state;
	
	data += curr;
	/* consume whitespace and separator */
	while (isspace(*data)) {
		++curr;
		++data;
	}
	if (*data == ':') {
		++curr;
	}
	return curr;
}
