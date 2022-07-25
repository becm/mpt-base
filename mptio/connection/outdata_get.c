/*!
 * MPT I/O library
 *   query parameters of output data
 */

#include <string.h>

#include "object.h"
#include "types.h"

#include "connection.h"

/*!
 * \ingroup mptConnection
 * \brief query outdata properties
 * 
 * Get properties of outdata.
 * 
 * \param od   output data descriptor
 * \param prop property data
 */
extern int mpt_outdata_get(const MPT_STRUCT(outdata) *od, MPT_STRUCT(property) *pr)
{
	const char *name;
	
	if (!pr) {
		return MPT_ENUM(TypeUnixSocket);
	}
	if ((name = pr->name) && !*name) {
		pr->name = "outdata";
		pr->desc = "output data context";
		MPT_property_set_data(pr, MPT_ENUM(TypeUnixSocket), &od->sock);
		return MPT_socket_active(&od->sock) ? 1 : 0;
	}
	return MPT_ERROR(BadArgument);
}
