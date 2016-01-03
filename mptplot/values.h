/*!
 * MPT plotting library
 *  value creation and processing
 */

#ifndef _MPT_VALUES_H
#define _MPT_VALUES_H 201401

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

MPT_INTERFACE(iterator)
#ifdef __cplusplus
{
	iterator();
	
	virtual int unref() = 0;
	virtual double next(int pos = 1) = 0;
	
	inline double current()
	{ return next(0); }
protected:
	inline ~iterator() { }
#else
; MPT_INTERFACE_VPTR(iterator) {
	int (*unref)(MPT_INTERFACE(iterator) *);
	double (*next)(MPT_INTERFACE(iterator) *, int);
}; MPT_INTERFACE(iterator) {
	MPT_INTERFACE_VPTR(iterator) *_vptr;
#endif
};

__MPT_EXTDECL_BEGIN

/* create profile of specific type */
extern int mpt_values_linear(int , double *, int , double , double);
extern int mpt_values_bound (int , double *, int , double , double , double);
extern int mpt_values_string(int , double *, int , const char *);
extern int mpt_values_poly  (int , double *, int , const char *, const double *);

/* prepare values on buffer offset */
extern double *mpt_values_prepare(MPT_STRUCT(array) *, int);

/* select/set solver profile type */
extern int mpt_valtype_select(const char *, char **);
extern int mpt_valtype_init(int , double *, int , const char *, int , const double *);

/* get to current/next (valid) entry */
extern double mpt_iterator_curr(MPT_INTERFACE(iterator) *);
extern int mpt_iterator_next(MPT_INTERFACE(iterator) *, double *);

/* create iterator (descr. includes type info) */
extern MPT_INTERFACE(iterator) *mpt_iterator_create(const char *);
/* create specific iterator */
extern MPT_INTERFACE(iterator) *_mpt_iterator_range (const char *);
extern MPT_INTERFACE(iterator) *_mpt_iterator_linear(const char *);
extern MPT_INTERFACE(iterator) *_mpt_iterator_values(const char *);
extern MPT_INTERFACE(iterator) *_mpt_iterator_factor(const char *);

#if defined(_STDIO_H) || defined(_STDIO_H_)
/* set solver matrix via file */
extern int mpt_conf_file(FILE *, int , int , double *);
#endif

/* create iterator */
extern int mpt_conf_iterator(MPT_INTERFACE(iterator) **, const MPT_STRUCT(node) *);
/* set matrix columns from stream according to mapping */
extern int mpt_conf_stream(MPT_STRUCT(array) * , void *, int , int);

/* create profile data */
extern int mpt_conf_profile(const MPT_STRUCT(array) *, int , int , const MPT_STRUCT(node) *, MPT_INTERFACE(logger) *);
extern int mpt_conf_profiles(int , double *, int , const MPT_STRUCT(node) *, const double *);

/* append user data */
extern int mpt_conf_param(MPT_STRUCT(array) *, const MPT_STRUCT(node) *, int);
/* set grid data */
extern int mpt_conf_grid(MPT_STRUCT(array) *, const MPT_STRUCT(node) *);

/* append values described by string */
extern double *mpt_conf_values(MPT_STRUCT(array) *, int , const char *);

__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_VALUES_H */
