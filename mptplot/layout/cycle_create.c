/*!
 * modify world cycle structure.
 */

#include <errno.h>
#include <stdlib.h>
#include <limits.h>

#include "array.h"
#include "layout.h"

struct Cycle
{
	MPT_INTERFACE(cycle) gen;
	MPT_STRUCT(array) arr;
	size_t act;
};

static int cycleUnref(MPT_INTERFACE(cycle) *wcyc)
{
	struct Cycle *cyc = (void *) wcyc;
	MPT_STRUCT(buffer) *buf;
	
	if ((buf = cyc->arr._buf)) {
		MPT_INTERFACE(polyline) **part = (void *) (buf+1);
		size_t	i, len = buf->used/sizeof(*part);
		
		for (i = 0; i < len; ++i) {
			MPT_INTERFACE(polyline) *curr = part[i];
			if (curr) {
				curr->_vptr->unref(curr);
			}
		}
		buf->used = 0;
		
		if (buf->resize) {
			buf->resize(buf, 0);
		}
	}
	cyc->arr._buf = 0;
	free(wcyc);
	return 0;
}
static MPT_INTERFACE(polyline) *cycleCurrent(const MPT_INTERFACE(cycle) *wcyc)
{
	const struct Cycle *cyc = (void *) wcyc;
	MPT_STRUCT(buffer) *buf;
	MPT_INTERFACE(polyline) **pos;
	size_t	len;
	
	if (!(buf = cyc->arr._buf)) return 0;
	if (!(len = buf->used/sizeof(*pos)) || len < cyc->act) return 0;
	pos = (void *) (buf+1);
	return pos[cyc->act];
}
static MPT_INTERFACE(polyline) *cycleAppend(MPT_INTERFACE(cycle) *wcyc)
{
	struct Cycle *cyc = (void *) wcyc;
	MPT_INTERFACE(polyline) **pos, *pl = 0;
	
	/* create new polyline */
	if (!(pl = mpt_pline_create())) {
		return 0;
	}
	if ((pos = mpt_array_append(&cyc->arr, sizeof(pl), &pl))) {
		return pl;
	}
	/* delete non-added polyline */
	pl->_vptr->unref(pl);
	
	return 0;
}
static MPT_INTERFACE(polyline) *cycleAdvance(MPT_INTERFACE(cycle) *wcyc)
{
	struct Cycle *cyc = (void *) wcyc;
	MPT_STRUCT(buffer) *buf;
	MPT_INTERFACE(polyline) **pos;
	size_t	len;
	
	if (!(buf = cyc->arr._buf)) {
		return 0;
	}
	if (!(len = buf->used/sizeof(*pos))) {
		return 0;
	}
	if (++cyc->act >= len) {
		cyc->act = 0;
	}
	pos = (void *) (buf+1);
	return pos[cyc->act];
}

static MPT_INTERFACE(polyline) *cyclePart(const MPT_INTERFACE(cycle) *wcyc, int pos)
{
	const struct Cycle *cyc = (void *) wcyc;
	MPT_STRUCT(buffer) *buf;
	size_t	len;
	
	if (!(buf = cyc->arr._buf)) return 0;
	
	len = buf->used/sizeof(void *);
	
	if (pos < 0) {
		if ((pos += len) < 0) return 0;
	}
	if ((unsigned int) pos >= len) {
		errno = ERANGE; return 0;
	}
	return ((MPT_INTERFACE(polyline) **) (buf+1))[pos];
}

static int cycleSize(const MPT_INTERFACE(cycle) *wcyc)
{
	const struct Cycle *cyc = (void *) wcyc;
	MPT_STRUCT(buffer) *buf;
	
	if (!(buf = cyc->arr._buf)) return 0;
	
	return buf->used/sizeof(void *);
}

static const MPT_INTERFACE_VPTR(cycle) _vptr = {
	cycleUnref,
	cycleCurrent, cycleAdvance, cycleAppend,
	cyclePart,
	cycleSize
};

/*!
 * \ingroup mptPlot
 * \brief simple cycle
 * 
 * Create simple cycle instance
 * 
 * \param pctl function to call for polyline creation/deletion
 * 
 * \return new cycle instance
 */
extern MPT_INTERFACE(cycle) *mpt_cycle_create(void)
{
	struct Cycle *cyc;
	
	if (!(cyc = malloc(sizeof(*cyc)))) {
		return 0;
	}
	cyc->gen._vptr = &_vptr;
	cyc->arr._buf = 0;
	cyc->act = 0;
	
	return &cyc->gen;
}
