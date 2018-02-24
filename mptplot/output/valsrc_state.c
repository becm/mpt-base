/*!
 * get binding source element
 */

#include <ctype.h>

#include "values.h"

/*!
 * \ingroup mptPlot
 * \brief set binding source
 * 
 * Parse single binding source parameter.
 * Call in loop to get all sources for description.
 * 
 * \param[in,out] src  value source
 * \param         data text describing binding sources
 * 
 * \return length of consumed text
 */
extern int mpt_valsrc_state(MPT_STRUCT(valsrc) *src, const char *data)
{
	uint8_t state;
	int curr;
	
	/* special validity for binding */
	if (!data || !*data) {
		src->state = MPT_DATASTATE(Step);
		return 0;
	}
	if (!isalpha(*data)) {
		src->state = MPT_DATASTATE(Step);
		return 0;
	}
	if (isalpha(data[1])) {
		return MPT_ERROR(BadValue);
	}
	switch (*data++) {
		case 'i': state = MPT_DATASTATE(Init); break;
		case 'I': state = MPT_DATASTATE(All) & ~MPT_DATASTATE(Init); break;
		case 's': state = MPT_DATASTATE(Step); break;
		case 'S': state = MPT_DATASTATE(All) & ~MPT_DATASTATE(Step); break;
		case 'f': state = MPT_DATASTATE(Fini); break;
		case 'F': state = MPT_DATASTATE(All) & ~MPT_DATASTATE(Fini); break;
		case 'A': state = 0; break;
		case 'a': state = MPT_DATASTATE(All); break;
		default:
			src->state = MPT_DATASTATE(Step);
			return 0;
	}
	src->state = state;
	curr = 1;
	while ((state = *(++data))) {
		if (!isspace(state)) {
			return curr;
		}
		++curr;
	}
	return curr;
}
