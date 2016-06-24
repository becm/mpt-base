/*!
 * clear references
 */

#include "array.h"

/*!
 * \ingroup mptArray
 * \brief dereference array elements
 * 
 * Assume array holds pointers elements who have
 *  1) virtual function pointer at beginning
 *  2) dereference function as first virtual function member.
 * 
 * This holds true for all MPT references types.
 * 
 * \param arr  array containing MPT references
 */
extern void mpt_array_callunref(const MPT_STRUCT(array) *a)
{
	MPT_STRUCT(buffer) *b;
	MPT_INTERFACE(logger) **m;
	size_t i, len;
	
	len = (b = a->_buf) ? b->used/sizeof(*m) : 0;
	m = (void *) (b+1);
	
	for (i = 0; i < len; ++i) {
		MPT_INTERFACE(logger) *c;
		if (!(c = m[i])) continue;
		c->_vptr->unref(c);
		m[i] = 0;
	}
}
