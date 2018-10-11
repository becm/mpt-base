/*
 * mpt C++ library
 *   destination operations
 */

#include "values.h"

__MPT_NAMESPACE_BEGIN

// layout target processing
bool laydest::match(laydest dst, int flg) const
{
	if (flg & MatchLayout
	 && dst.lay != lay) {
		return false;
	}
	if (flg & MatchGraph
	 && dst.grf != grf) {
		return false;
	}
	if (flg & MatchWorld
	 && dst.wld != wld) {
		return false;
	}
	if (flg & MatchDimension
	 && dst.dim != dim) {
		return false;
	}
	return true;
}

__MPT_NAMESPACE_END
