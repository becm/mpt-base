/*!
 * set history state
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "array.h"
#include "message.h"

#include "output.h"

/*!
 * \ingroup mptOutput
 * \brief set history parameters
 * 
 * Add binding to history info.
 * Pass zero pointer to reset.
 * 
 * \param hist  history state
 * \param bnd   binding to add
 * 
 * \return zero on success
 */
extern int mpt_history_set(MPT_STRUCT(histinfo) *hist, const MPT_STRUCT(msgvalfmt) *bnd)
{
	size_t part;
	uint8_t type, size;
	
	if (!bnd) {
		hist->pos  = 0;
		hist->part = 0;
		hist->line = 0;
		hist->type = 0;
		hist->size = 0;
		return 0;
	}
	type = bnd->fmt;
	size = type & 0x3f;
	part = size * bnd->len;
	
	/* bad element size */
	if (!(type & 0x3f)) {
		return -1;
	}
	/* size overflow */
	if ((part + hist->part) > UINT16_MAX) {
		return -2;
	}
	/* check element type */
	switch (type & 0x7f) {
	  /* floating point values */
	  case MPT_ENUM(ValuesFloat) | sizeof(float) : type = 'f'; break;
	  case MPT_ENUM(ValuesFloat) | sizeof(double): type = 'd'; break;
	  /* unsigned integer values */
	  case MPT_ENUM(ValuesUnsigned) | sizeof(int8_t):  type = 'y'; break;
	  case MPT_ENUM(ValuesUnsigned) | sizeof(int16_t): type = 'q'; break;
	  case MPT_ENUM(ValuesUnsigned) | sizeof(int32_t): type = 'u'; break;
	  case MPT_ENUM(ValuesUnsigned) | sizeof(int64_t): type = 't'; break;
	  /* signed integer values */
	  case sizeof(int8_t):  type = 'b'; break;
	  case sizeof(int16_t): type = 'n'; break;
	  case sizeof(int32_t): type = 'i'; break;
	  case sizeof(int64_t): type = 'x'; break;
	  default: return -3;
	}
	/* initial part */
	if (!hist->line) {
		hist->type = type;
		hist->size = size;
		hist->line = hist->part = part;
		return part;
	}
	/* add to visible size */
	if ((hist->part == hist->line) && (type == hist->type)) {
		hist->line += part;
	}
	hist->part += part;
	
	return part;
}
