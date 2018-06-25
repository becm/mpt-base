/*!
 * set format according to string parameter.
 */

#include <string.h>
#include <ctype.h>

#include "parse.h"


/*!
 * \ingroup mptParse
 * \brief parser format setup
 * 
 * initialize parser format according to string description.
 * 
 * \param fmt format information
 * \param str input format description
 * 
 * \return parse format type
 */

extern int mpt_parse_format(MPT_STRUCT(parser_format) *fmt, const char *str)
{
	static const MPT_STRUCT(parser_format) pfmt_default = MPT_PARSER_FORMAT_INIT;
	
	int i = pfmt_default.sstart, ret = '*';
	
	if (!str) {
		*fmt = pfmt_default;
		return ret;
	}
	/* section start delimiter */
	if (str[0]) {
		i = isspace(str[0]) ? 0 : str[0];
		if (str[1]) ret = str[1];
	}
	/* select format type */
	*fmt = pfmt_default;
	fmt->sstart = i;
	
	/* separate delimiter */
	if (str[2]) fmt->send = isspace(str[2]) ? 0 : str[2];
	else return ret;
	
	if (str[3]) fmt->ostart = isspace(str[3]) ? 0 : str[3];
	else return ret;
	
	if (str[4]) fmt->assign = isspace(str[4]) ? 0 : str[4];
	else return ret;
	
	if (str[5]) fmt->oend = isspace(str[5]) ? 0 : str[5];
	else return ret;
	
	str += 6;
	if (!*str) {
		return ret;
	}
	
	/* comments until space character */
	for (i = 0; *str && !isspace(*str) && i < (int) sizeof(fmt->com); ++str, ++i) {
		fmt->com[i] = *str;
	}
	for ( ; i < (int) sizeof(fmt->com); ++i) {
		fmt->com[i] = 0;
	}
	/* consume whitespace */
	while (*str && isspace(*str)) {
		++str;
	}
	if (!*str) {
		return ret;
	}
	/* escape character after comments */
	for (i = 0; *str && !isspace(*str) && i < (int) sizeof(fmt->esc); ++str, ++i) {
		fmt->esc[i] = *str;
	}
	for ( ; i < (int) sizeof(fmt->esc); ++i) {
		fmt->esc[i] = 0;
	}
	return ret;
}
