/*!
 * set line attributes.
 */

#include "meta.h"

#include "layout.h"

#define MPT_LineStyleMax   5
#define MPT_LineWidthMax   10
#define MPT_SymbolTypeMax  8
#define MPT_SymbolSizeMax  20

/*!
 * \ingroup mptPlot
 * \brief set line attributes
 * 
 * Set values for line attribute members.
 * Use -1 for default value.
 * 
 * \param attr   line attribute data
 * \param width  line width
 * \param style  line stroke style
 * \param symbol line point symbols
 * \param size   symbols size
 */
extern int mpt_lattr_set(MPT_STRUCT(lineattr) *attr, int width, int style, int symbol, int size)
{
	if (!attr) {
		return MPT_ERROR(BadArgument);
	}
	if (width  > MPT_ENUM(LineWidthMax) ) return MPT_ERROR(BadValue);
	if (style  > MPT_ENUM(LineStyleMax) ) return MPT_ERROR(BadValue);
	if (symbol > MPT_ENUM(SymbolTypeMax)) return MPT_ERROR(BadValue);
	if (size   > MPT_ENUM(SymbolSizeMax)) return MPT_ERROR(BadValue);
	
	attr->width  = width  >= 0 ? width  : 1;
	attr->style  = style  >= 0 ? style  : 1;
	attr->symbol = symbol >= 0 ? symbol : 0;
	attr->size   = size   >= 0 ? size   : 10;
	
	return 0;
}

static int lattr_pset(unsigned char *val, MPT_INTERFACE(convertable) *src, int def[3])
{
	int len;
	uint8_t sym;
	
	if (!src) {
		*val = def[0];
		return 0;
	}
	if ((len = src->_vptr->convert(src, 'y', &sym)) < 0) {
		int32_t tmp = *val;
		if ((len = src->_vptr->convert(src, 'i', &tmp)) < 0) {
			return len;
		}
		if (tmp < 0 || tmp > UINT8_MAX) {
			return MPT_ERROR(BadValue);
		}
		sym = tmp;
	}
	if (!len) {
		*val = def[0];
		return 0;
	}
	if (sym < def[1] || sym > def[2]) {
		return MPT_ERROR(BadValue);
	}
	*val = sym;
	
	return 0;
}

/*!
 * \ingroup mptPlot
 * \brief set symbol attribute
 * 
 * Get symbol attribute from data source.
 * 
 * \param attr line attributes
 * \param src  data source
 * 
 * \return consumed data length
 */
extern int mpt_lattr_symbol(MPT_STRUCT(lineattr) *attr, MPT_INTERFACE(convertable) *src)
{
	int def[3] = { 0, 0, MPT_ENUM(SymbolTypeMax) };
	return lattr_pset(&attr->symbol, src, def);
}
/*!
 * \ingroup mptPlot
 * \brief set size attribute
 * 
 * Get size attribute from data source.
 * 
 * \param attr line attributes
 * \param src  data source
 * 
 * \return consumed data length
 */
extern int mpt_lattr_size(MPT_STRUCT(lineattr) *attr, MPT_INTERFACE(convertable) *src)
{
	int def[3] = { 10, 0, MPT_ENUM(SymbolSizeMax) };
	return lattr_pset(&attr->size, src, def);
}
/*!
 * \ingroup mptPlot
 * \brief set style attribute
 * 
 * Get style attribute from data source.
 * 
 * \param attr line attributes
 * \param src  data source
 * 
 * \return consumed data length
 */
extern int mpt_lattr_style(MPT_STRUCT(lineattr) *attr, MPT_INTERFACE(convertable) *src)
{
	int def[3] = { 1, 0, MPT_ENUM(LineStyleMax) };
	return lattr_pset(&attr->style, src, def);
}
/*!
 * \ingroup mptPlot
 * \brief set width attribute
 * 
 * Get width attribute from data source.
 * 
 * \param attr line attributes
 * \param src  data source
 * 
 * \return consumed data length
 */
extern int mpt_lattr_width(MPT_STRUCT(lineattr) *attr, MPT_INTERFACE(convertable) *src)
{
	int def[3] = { 1, 0, MPT_ENUM(LineWidthMax) };
	return lattr_pset(&attr->width, src, def);
}

