/*!
 * set name restrictions from description string.
 */

#include <ctype.h>

#include "parse.h"

/*!
 * \ingroup mptParse
 * \brief set accepted characters
 * 
 * Use string to describe accepted characters
 * in section and option names.
 * 
 * \param flg  parser flags
 * \param name description for accepted characters
 * 
 * \return parser code for next element
 */
extern int mpt_parse_accept(MPT_STRUCT(parser_allow) *flg, const char *name)
{
	const char *end = name;
	uint8_t sect = 0, opt = 0;
	
	if (!end) {
		flg->sect = flg->opt = 0xff;
		return 0;
	}
	if (!*end) {
		flg->sect = flg->opt = MPT_NAMEFLAG(NumCont);
		return 0;
	}
	while (*end) {
		int type;
		
		if (isspace(*end)) {
			break;
		}
		switch (type = tolower(*end)) {
		  case 'f': type = MPT_NAMEFLAG(NumStart); break;
		  case 'c': type = MPT_NAMEFLAG(NumCont); break;
		  case 'n': type = MPT_NAMEFLAG(Numeral); break;
		  case 's': type = MPT_NAMEFLAG(Special); break;
		  case 'w': type = MPT_NAMEFLAG(Space); break;
		  
		  case 'e': type = MPT_NAMEFLAG(Empty); break;
		  case 'b': type = MPT_NAMEFLAG(Binary); break;
		  default: return -2;
		}
		if (isupper(*end)) {
			sect |= type;
		} else {
			opt |= type;
		}
		++end;
	}
	flg->sect = sect;
	flg->opt  = opt;
	
	return end - name;
}
