/*!
 * get binding source element
 */

#include <ctype.h>

#include "convert.h"
#include "message.h"

#include "output.h"

/*!
 * \ingroup mptOutput
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
extern int mpt_outbind_set(MPT_STRUCT(msgbind) *bnd, const char *data)
{
	uint8_t val, type;
	int	curr;
	
	/* special validity for binding */
	switch (data[0]) {
		case 'i': type = MPT_ENUM(OutputStateInit); break;
		case 'I': type = MPT_ENUM(OutputStates) & ~MPT_ENUM(OutputStateInit); break;
		case 's': type = MPT_ENUM(OutputStateStep); break;
		case 'S': type = MPT_ENUM(OutputStates) & ~MPT_ENUM(OutputStateStep); break;
		case 'f': type = MPT_ENUM(OutputStateFini); break;
		case 'F': type = MPT_ENUM(OutputStates) & ~MPT_ENUM(OutputStateFini); break;
		case 'A': type = 0; break;
		case 'a':
		default:  type = MPT_ENUM(OutputStates);
	}
	if (isalpha(data[0])) {
		++data;
	}
	if ((curr = mpt_cuint8(&val, data, 0, 0)) <= 0) {
		return curr;
	}
	bnd->type = type;
	bnd->dim  = val;
	data += curr;
	while (isspace(*data)) { ++curr; ++data; }
	if (*data == ':') ++curr;
	
	return curr;
}
