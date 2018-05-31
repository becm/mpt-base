/*!
 * MPT plotting library
 *  history file data output
 */

#ifndef _MPT_HISTORY_H
#define _MPT_HISTORY_H  @INTERFACE_VERSION@

#include "array.h"
#include "output.h"

/* history format information */
MPT_STRUCT(histfmt)
{
#ifdef __cplusplus
public:
	inline histfmt() : pos(0), fmt(0)
	{ }
	bool set_format(const char *fmt);
	bool add(valfmt);
	bool add(char);
protected:
#else
# define MPT_HISTFMT_INIT  { MPT_ARRAY_INIT, MPT_ARRAY_INIT, 0,0 }
#endif
	_MPT_ARRAY_TYPE(valfmt) _fmt;  /* output format */
	_MPT_ARRAY_TYPE(char)   _dat;  /* data format */
	
	uint8_t pos;  /* element position */
	char    fmt;  /* data format */
};

#ifdef __cplusplus
struct history : logfile, public histfmt
{
#else
MPT_STRUCT(history)
{
# define MPT_HISTORY_INIT { MPT_LOGFILE_INIT, MPT_HISTFMT_INIT }
	MPT_STRUCT(logfile) info;
	MPT_STRUCT(histfmt) fmt;
#endif
};

__MPT_EXTDECL_BEGIN

/* reset history output state */
void mpt_histfmt_reset(MPT_STRUCT(histfmt) *);

/* clear history resources */
extern void mpt_history_fini(MPT_STRUCT(history) *);
/* get/set history properties */
extern int mpt_history_get(const MPT_STRUCT(history) *, MPT_STRUCT(property) *);
extern int mpt_history_set(MPT_STRUCT(history) *, const char *, const MPT_INTERFACE(metatype) *);

/* push data to history */
extern ssize_t mpt_history_push(MPT_STRUCT(history) *, size_t , const void *);
/* data print setup and processing */
extern ssize_t mpt_history_values(MPT_STRUCT(history) *, size_t , const void *);

__MPT_NAMESPACE_END

#endif /* _MPT_HISTORY_H */
