
#include <errno.h>
#include <stddef.h>

#include "config.h"
#include "parse.h"

static int parseCheck(void *data, const MPT_STRUCT(path) *path, int op)
{
	const MPT_STRUCT(parseflg) *p = data;
	const char *name;
	
	switch (op & 0x3) {
	    case MPT_ENUM(ParseSection):
		op = p->sect;
		break;
	    case MPT_ENUM(ParseOption):
		op = p->opt;
		break;
	    default:
		return 0;
	}
	name = path->base + path->off + path->len;
	return mpt_parse_ncheck(name, path->valid, op);
}
/*!
 * \ingroup mptParse
 * \brief parser setup
 * 
 * initialize parser descriptor to default values.
 * 
 * \param parse parser structure
 * \param fmt   format information
 */
extern void mpt_parse_init(MPT_STRUCT(parse) *parse)
{
	parse->source.getc = mpt_getchar_file;
	parse->source.arg = 0;
	parse->line = 1;
	
	parse->check.ctl = parseCheck;
	parse->check.arg = &parse->name;
	
	mpt_parse_format(&parse->format, 0);
	
	parse->lastop = 0;
	
	parse->name.sect = 0xff;
	parse->name.opt  = 0xff;
}
