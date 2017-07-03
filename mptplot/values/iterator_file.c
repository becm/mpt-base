/*!
 * create iterator with constant difference between elements.
 */

#ifndef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 1
#endif

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <inttypes.h>

#include "convert.h"
#include "meta.h"
#include "parse.h"

#include "values.h"

struct _iter_fdata
{
	FILE *fd;
	int type;
	uint8_t val[16];
};

static void iterUnref(MPT_INTERFACE(unrefable) *ref)
{
	struct _iter_fdata *d = (void *) (ref + 1);
	fclose(d->fd);
	free(ref);
}
static int iterGet(MPT_INTERFACE(iterator) *it, int t, void *ptr)
{
	struct _iter_fdata *d = (void *) (it + 1);
	const void *src;
	int ret;
	
	if (!t) {
		MPT_STRUCT(value) *val;
		static const char fmt[] = { MPT_ENUM(TypeFile), 0 };
		if ((val = ptr)) {
			val->fmt = fmt;
			val->ptr = d;
		}
		return 0;
	}
	if (!d->type) {
		size_t len;
		switch (t) {
		  case 'd': ret = fscanf(d->fd, "%lf",      (double *)   d->val); len = sizeof(double);   break;
		  case 'f': ret = fscanf(d->fd, "%f",       (float *)    d->val); len = sizeof(float);    break;
		  case 't': ret = fscanf(d->fd, "%" SCNu64, (uint64_t *) d->val); len = sizeof(uint64_t); break;
		  case 'x': ret = fscanf(d->fd, "%" SCNi64, (int64_t *)  d->val); len = sizeof(int64_t);  break;
		  case 'u': ret = fscanf(d->fd, "%" SCNu32, (uint32_t *) d->val); len = sizeof(uint32_t); break;
		  case 'i': ret = fscanf(d->fd, "%" SCNi32, (int32_t *)  d->val); len = sizeof(int32_t);  break;
		  case 'q': ret = fscanf(d->fd, "%" SCNu16, (uint16_t *) d->val); len = sizeof(uint16_t); break;
		  case 'n': ret = fscanf(d->fd, "%" SCNi16, (int16_t *)  d->val); len = sizeof(int16_t);  break;
# if __STDC_VERSION__ >= 199901L
		  case 'y': ret = fscanf(d->fd, "%" SCNu8,  (uint8_t *)  d->val); len = sizeof(uint8_t);  break;
		  case 'b': ret = fscanf(d->fd, "%" SCNi8,  (int8_t *)   d->val); len = sizeof(int8_t);   break;
#endif
		  case 'c': ret = fscanf(d->fd, "%c",       (char *)     d->val); len = sizeof(char);     break;
		  default: return MPT_ERROR(BadType);
		}
		if (!ret) {
			d->type = -1;
			return 0;
		}
		if (ret < 0) {
			return MPT_ERROR(BadValue);
		}
		if (ptr) {
			memcpy(ptr, d->val, len);
		}
		d->type = t;
		return MPT_ENUM(TypeFile);
	}
	else if (d->type < 0) {
		return 0;
	}
	src = d->val;
	if ((ret = mpt_data_convert(&src, d->type, ptr, t)) < 0) {
		return ret;
	}
	return d->type;
}
static int iterAdvance(MPT_INTERFACE(iterator) *it)
{
	struct _iter_fdata *d = (void *) (it + 1);
	if (d->type < 0) {
		return MPT_ERROR(MissingData);
	}
	if (!d->type) {
		int ret;
		if ((ret = iterGet(it, 'd', 0)) < 0) {
			return ret;
		}
		return 0;
	}
	d->type = 0;
	return MPT_ENUM(TypeFile);
}
static int iterReset(MPT_INTERFACE(iterator) *it)
{
	struct _iter_fdata *d = (void *) (it + 1);
	rewind(d->fd);
	d->type = 0;
	return 0;
}
static const MPT_INTERFACE_VPTR(iterator) iteratorFile = {
	{ iterUnref },
	iterGet,
	iterAdvance,
	iterReset
};

/*!
 * \ingroup mptValues
 * \brief create file iterator
 * 
 * Create iterator with file descriptor source.
 * 
 * \param fd  file id
 * 
 * \return file iterator
 */
extern MPT_INTERFACE(iterator) *mpt_iterator_file(int fd)
{
	MPT_INTERFACE(iterator) *iter;
	struct _iter_fdata *data;
	FILE *file;
	
	if ((file = fdopen(fd, "r"))) {
		return 0;
	}
	if (!(iter = malloc(sizeof(*iter) + sizeof(*data)))) {
		return 0;
	}
	data = (void *) (iter + 1);
	iter->_vptr = &iteratorFile;
	data->fd = file;
	data->type = 0;
	
	return iter;
}

