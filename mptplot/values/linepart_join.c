/*!
 * merge visibility information
 */

#include <errno.h>
#include <string.h>
#include <limits.h>

#include "values.h"

/*!
 * \ingroup mptPlot
 * \brief join line part segments
 * 
 * Try to merge second line part with first.
 * 
 * \param to	first line part
 * \param post	second line part
 * 
 * \return first line part on success
 */
extern MPT_STRUCT(linepart) *mpt_linepart_join(MPT_STRUCT(linepart) *to, MPT_STRUCT(linepart) post)
{
	if ((UINT16_MAX - to->raw) < post.raw) return 0;
	if ((UINT16_MAX - to->usr) < post.usr) return 0;
	
	if (to->_trim || post._cut || to->usr != to->raw) return 0;
	
	to->raw += post.raw;
	to->usr += post.usr;
	
	return to;
}
