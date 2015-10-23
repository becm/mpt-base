/*!
 * set line attributes.
 */

#include <errno.h>

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
 * \param graph	uninitialized graph data
 */
extern int mpt_lattr_set(MPT_STRUCT(lineattr) *attr, int width, int style, int symbol, int size)
{
	if (!attr) {
		errno = EFAULT; return -1;
	}
	if (width  > MPT_ENUM(LineWidthMax) ) return -2;
	if (style  > MPT_ENUM(LineStyleMax) ) return -2;
	if (symbol > MPT_ENUM(SymbolTypeMax)) return -2;
	if (size   > MPT_ENUM(SymbolSizeMax)) return -2;
	
	attr->width  = width  >= 0 ? width  : 1;
	attr->style  = style  >= 0 ? style  : 1;
	attr->symbol = symbol >= 0 ? symbol : 0;
	attr->size   = size   >= 0 ? size   : 10;
	
	return 0;
}

static int lattr_pset(unsigned char *val, MPT_INTERFACE(source) *src, int def[3])
{
	int	len, sym;
	
	if (!src)
		return (*val != *def) ? 1 : 0;
	
	if ((len = src->_vptr->conv(src, 'i', &sym)) < 0)
		return len;
	
	if (!len) { *val = def[0]; return 0; }
	
	if (sym < def[1] || sym > def[2]) {
		errno = ERANGE; return -1;
	}
	*val = sym;
	
	return len;
}

/*!
 * \ingroup mptPlot
 * \brief set symbol attribute
 * 
 * Get symbol attribute from data source.
 * 
 * \param attr	line attributes
 * \param src	data source
 * 
 * \return consumed data length
 */
extern int mpt_lattr_symbol(MPT_STRUCT(lineattr) *attr, MPT_INTERFACE(source) *src)
{
	int	def[3] = { 0, 0, MPT_ENUM(SymbolTypeMax) };
	return lattr_pset(&attr->symbol, src, def);
}
/*!
 * \ingroup mptPlot
 * \brief set size attribute
 * 
 * Get size attribute from data source.
 * 
 * \param attr	line attributes
 * \param src	data source
 * 
 * \return consumed data length
 */
extern int mpt_lattr_size(MPT_STRUCT(lineattr) *attr, MPT_INTERFACE(source) *src)
{
	int	def[3] = { 10, 0, MPT_ENUM(SymbolSizeMax) };
	return lattr_pset(&attr->size, src, def);
}
/*!
 * \ingroup mptPlot
 * \brief set style attribute
 * 
 * Get style attribute from data source.
 * 
 * \param attr	line attributes
 * \param src	data source
 * 
 * \return consumed data length
 */
extern int mpt_lattr_style(MPT_STRUCT(lineattr) *attr, MPT_INTERFACE(source) *src)
{
	int	def[3] = { 1, 0, MPT_ENUM(LineStyleMax) };
	return lattr_pset(&attr->style, src, def);
}
/*!
 * \ingroup mptPlot
 * \brief set width attribute
 * 
 * Get width attribute from data source.
 * 
 * \param attr	line attributes
 * \param src	data source
 * 
 * \return consumed data length
 */
extern int mpt_lattr_width(MPT_STRUCT(lineattr) *attr, MPT_INTERFACE(source) *src)
{
	int	def[3] = { 1, 0, MPT_ENUM(LineWidthMax) };
	return lattr_pset(&attr->width, src, def);
}

