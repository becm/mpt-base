/*!
 * MPT plotting library
 *  value creation and processing
 */

#ifndef _MPT_VALUES_H
#define _MPT_VALUES_H  @INTERFACE_VERSION@

#include "core.h"

#ifdef __cplusplus
# include "array.h"
#endif

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(node);
MPT_STRUCT(array);


enum MPT_ENUM(ValueDescription)
{
	MPT_ENUM(ValueFormatLinear) = 1,
	MPT_ENUM(ValueFormatBoundaries),
	MPT_ENUM(ValueFormatText),
	MPT_ENUM(ValueFormatPolynom),
	MPT_ENUM(ValueFormatFile)
};

__MPT_EXTDECL_BEGIN

/* create profile of specific type */
extern int mpt_values_linear(int , double *, int , double , double);
extern int mpt_values_bound (int , double *, int , double , double , double);
extern int mpt_values_string(const char *, int , double *, int);
extern int mpt_values_poly  (const char *, int , double *, int , const double *);

/* prepare values on buffer offset */
extern double *mpt_values_prepare(MPT_STRUCT(array) *, int);

/* select/set solver profile type */
extern int mpt_valtype_select(const char **);
extern int mpt_valtype_init(int , const char *, int , double *, int , const double *);

/* create iterator (descr. includes type info) */
extern MPT_INTERFACE(metatype) *mpt_iterator_create(const char *);
/* create specific iterator */
extern MPT_INTERFACE(metatype) *_mpt_iterator_range (const char *);
extern MPT_INTERFACE(metatype) *_mpt_iterator_linear(const char *);
extern MPT_INTERFACE(metatype) *_mpt_iterator_values(const char *);
extern MPT_INTERFACE(metatype) *_mpt_iterator_factor(const char *);

#if defined(_STDIO_H) || defined(_STDIO_H_)
/* set solver matrix via file */
extern int mpt_conf_file(FILE *, int , int , double *);
#endif

/* set matrix columns from stream according to mapping */
extern int mpt_conf_stream(MPT_STRUCT(array) * , void *, int , int);

/* create profile data */
extern int mpt_conf_profiles(int , double *, int , const MPT_STRUCT(node) *, const double *, MPT_INTERFACE(logger) *);
extern int mpt_conf_profile (int , double *, int , const MPT_STRUCT(node) *, const double *);

/* append user data */
extern int mpt_conf_param(MPT_STRUCT(array) *, const MPT_STRUCT(node) *, int);
/* set grid data */
extern int mpt_conf_grid(MPT_STRUCT(array) *, const MPT_STRUCT(node) *);

/* append values described by string */
extern double *mpt_conf_values(MPT_STRUCT(array) *, int , const char *);

__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_VALUES_H */
