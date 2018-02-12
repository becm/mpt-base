/*!
 * modify data bindings
 */

#include "array.h"
#include "message.h"

#include "layout.h"

/*!
 * \ingroup mptOutput
 * \brief add mapping
 * 
 * Register mapping to array.
 * 
 * \param arr  array descriptor
 * \param src  data source
 * \param dst  layout target
 * \param cli  source client
 * 
 * \retval -3  conflicting binding
 * \retval -1  append failed
 * \retval 0   binding appended
 * \retval >0  reused element
 */
extern int mpt_mapping_add(MPT_STRUCT(array) *arr, const MPT_STRUCT(mapping) *add)
{
	MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(mapping) *map;
	size_t i, len = 0;
	
	if ((buf = arr->_buf)) {
		len = buf->_used / sizeof(*map);
		map = (void *) (buf+1);
	} else {
		len = 0;
		map = 0;
	}
	/* binding matches existing */
	for (i = 0; i < len; ++i) {
		/* binding not mergeable */
		if (map[i].client != add->client
		    || (map[i].dest.lay == add->dest.lay && map[i].dest.grf == add->dest.grf
		     && map[i].dest.wld == add->dest.wld && map[i].dest.dim == add->dest.dim)) {
			continue;
		}
		/* conflicting sources */
		if (map[i].src.dim != add->src.dim) {
			return -3;
		}
		/* add mapping states */
		map[i].src.state |= add->src.state;
		return i+1;
	}
	if (!(map = mpt_array_append(arr, sizeof(*map), 0))) {
		return -1;
	}
	*map = *add;
	
	return 0;
}
/*!
 * \ingroup mptOutput
 * \brief delete mapping
 * 
 * Unegister mapping in array.
 * 
 * \param arr  array descriptor
 * \param src  data source
 * \param dst  layout target
 * \param cli  source client
 */
extern int mpt_mapping_del(const MPT_STRUCT(array) *arr, const MPT_STRUCT(msgbind) *src, const MPT_STRUCT(laydest) *dst, int cli)
{
	MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(mapping) *map;
	size_t i, len = 0, del = 0;
	
	if (!(buf = arr->_buf)) {
		return 0;
	}
	len = buf->_used / sizeof(*map);
	map = (void *) (buf + 1);
	
	/* binding matches existing */
	for (i = 0; i < len; ++i) {
		/* binding not mergeable */
		if (map[i].client != cli) {
			continue;
		}
		if (dst
		    && (map[i].dest.lay != dst->lay || map[i].dest.grf != dst->grf
		     || map[i].dest.wld != dst->wld || map[i].dest.dim != dst->dim)) {
			continue;
		}
		if (!src) {
			map[i].src.state = 0;
		}
		else if (map[i].src.dim != src->dim) {
			continue;
		}
		else {
			map[i].src.state &= ~src->state;
		}
		++del;
	}
	if (del) {
		MPT_STRUCT(mapping) *empty;
		size_t used;
		
		for (i = 0, used = 0, empty = 0; i < len; ++i) {
			if (!map[i].src.state) {
				if (!empty) {
					empty = map+i;
				}
				continue;
			}
			/* track number of active mappings */
			++used;
			
			/* no smaller position available */
			if (!empty) continue;
			
			/* move command entry */
			*empty = map[i];
			map[i].src.state = 0;
			
			/* find smallest free position */
			while (++empty < (map+i)) {
				if (!empty->src.state) {
					break;
				}
			}
		}
		buf->_used = used * sizeof(*map);
	}
	return del;
}
/*!
 * \ingroup mptOutput
 * \brief compare mapping
 * 
 * Check mapping for source data match.
 * 
 * \param map  mapping data
 * \param src  data source
 * \param cli  source client
 */
int mpt_mapping_cmp(const MPT_STRUCT(mapping) *map, const MPT_STRUCT(msgbind) *src, int cli)
{
	if (cli >= 0 && (map->client != cli)) {
		return -1;
	}
	if (map->src.dim != src->dim) {
		return -2;
	}
	if (map->src.state & src->state) {
		return 0;
	}
	return 1;
}
