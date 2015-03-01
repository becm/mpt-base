
#include "parse.h"

/*!
 * \ingroup mptParse
 * \brief next element selection
 * 
 * select next element parser function for specified format.
 * 
 * \param fmt	format information
 * 
 * \return function to get next element
 */

extern MPT_TYPE(ParserFcn) mpt_parse_next_fcn(int fmt)
{
	/* select format type */
	switch (fmt) {
	case '_': return mpt_parse_option;     break;
	case 'x': return mpt_parse_format_enc; break;
	case ' ': return mpt_parse_format_sep; break;
	case '*': return mpt_parse_format_pre; break;
	default: return 0;
	}
}
