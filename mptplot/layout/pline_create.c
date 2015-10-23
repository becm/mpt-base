/*!
 * basic world part implementation.
 */

#include <errno.h>
#include <stdlib.h>
#include <limits.h>

#include "array.h"
#include "layout.h"

struct pl3Data
{
	MPT_INTERFACE(polyline) pt;
	MPT_STRUCT(array) x, y, z;
};

static int pl3_unref(MPT_INTERFACE(polyline) *part)
{
	struct pl3Data *pt = (void *) part;
	
	mpt_array_clone(&pt->x, 0);
	mpt_array_clone(&pt->y, 0);
	mpt_array_clone(&pt->z, 0);
	
	free(pt);
	
	return 0;
}
static ssize_t pl3_truncate(MPT_INTERFACE(polyline) *part, int dim, ssize_t len)
{
	struct pl3Data *pt = (void *) part;
	MPT_STRUCT(array) *a;
	
	if (dim < 0) {
		len = mpt_pline_truncate(&pt->x, 3, sizeof(double) * len);
		return (len < 0) ? len : (len /= sizeof(double));
	}
	switch (dim) {
	    case 0: a = &pt->x; break;
	    case 1: a = &pt->y; break;
	    case 2: a = &pt->z; break;
	    default: return -1;
	}
	if (len < 0) {
		return a->_buf ? a->_buf->used/sizeof(double) : 0;
	}
	else if (!mpt_array_slice(a, 0, len*sizeof(double))) {
		return -1;
	}
	a->_buf->used = len * sizeof(double);
	
	return len;
}
static void *pl3_raw(MPT_INTERFACE(polyline) *part, int dim, size_t need, size_t off)
{
	struct pl3Data *pt = (void *) part;
	MPT_STRUCT(array) *arr;
	double	*ptr;
	size_t	len, total;
	
	switch (dim) {
	    case 0: arr = &pt->x; break;
	    case 1: arr = &pt->y; break;
	    case 2: arr = &pt->z; break;
	    default: return 0;
	}
	len = arr->_buf ? arr->_buf->used / sizeof(double) : 0;
	ptr = (double *) (arr->_buf+1);
	
	if (need && len < (total = need + off)) {
		if (!(ptr = mpt_array_append(arr, (total-len)*sizeof(double), 0)))
			return 0;
	}
	return ptr + off;
}
static const char *pl3_format(const MPT_INTERFACE(polyline) *part)
{
	struct pl3Data *pt = (void *) part;
	static const char fmt[4] = "ddd";
	return pt->z._buf ? fmt : fmt + 1;
}

static MPT_INTERFACE_VPTR(polyline) wpctl = {
	pl3_unref,
	pl3_raw,
	pl3_truncate,
	pl3_format
};

/*!
 * \ingroup mptPlot
 * \brief create simple polyline
 * 
 * Basic polyline for (up to) 3D double data storage only.
 * 
 * \return polyline instance
 */
extern MPT_INTERFACE(polyline) *mpt_pline_create(void)
{
	struct pl3Data *pt;
	
	if (!(pt = malloc(sizeof(*pt)))) {
		return 0;
	}
	pt->pt._vptr = &wpctl;
	pt->x._buf = pt->y._buf = pt->z._buf = 0;
	
	return &pt->pt;
}

