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
	int count;
	uint8_t val[16];
};

/* convertable interface */
static int fileConv(MPT_INTERFACE(convertable) *val, int type, void *ptr)
{
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeIterator), MPT_ENUM(TypeFile) };
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
			return 0;
		}
		return MPT_ENUM(TypeIterator);
	}
	if (type == MPT_type_pointer(MPT_ENUM(TypeIterator))) {
		if (ptr) *((const void **) ptr) = val + 1;
		return MPT_ENUM(TypeFile);
	}
	if (type == MPT_type_pointer(MPT_ENUM(TypeFile))) {
		struct _iter_fdata *d = (void *) (val + 2);
		if (ptr) *((void **) ptr) = d->fd;
		return MPT_ENUM(TypeIterator);
	}
	return MPT_ERROR(BadType);
}
/* metatype interface */
static void fileUnref(MPT_INTERFACE(metatype) *mt)
{
	struct _iter_fdata *d = (void *) (mt + 2);
	fclose(d->fd);
	free(mt);
}
static uintptr_t fileRef(MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
static MPT_INTERFACE(metatype) *fileClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* iterator interface */
static int fileGet(MPT_INTERFACE(iterator) *it, int type, void *ptr)
{
	struct _iter_fdata *d = (void *) (it + 1);
	const void *src;
	int ret;
	
	if (!type) {
		static const uint8_t fmt[] = "dftxuiqnybc";
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
			return d->type;
		}
		return d->count;
	}
	if (!d->type) {
		size_t len;
		switch (type) {
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
		d->type = type;
		return MPT_ENUM(TypeFile);
	}
	else if (d->type < 0) {
		return 0;
	}
	src = d->val;
	if ((ret = mpt_data_convert(&src, d->type, ptr, type)) < 0) {
		return ret;
	}
	return d->type;
}
static int fileAdvance(MPT_INTERFACE(iterator) *it)
{
	struct _iter_fdata *d = (void *) (it + 1);
	if (d->type < 0) {
		return MPT_ERROR(MissingData);
	}
	if (!d->type) {
		int ret;
		if ((ret = fileGet(it, 'd', 0)) < 0) {
			return ret;
		}
		++d->count;
		return 0;
	}
	d->type = 0;
	return MPT_ENUM(TypeFile);
}
static int fileReset(MPT_INTERFACE(iterator) *it)
{
	struct _iter_fdata *d = (void *) (it + 1);
	rewind(d->fd);
	d->type = 0;
	d->count = 0;
	return 0;
}

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
extern MPT_INTERFACE(metatype) *mpt_iterator_file(int fd)
{
	static const MPT_INTERFACE_VPTR(metatype) fileMeta = {
		{ fileConv },
		fileUnref,
		fileRef,
		fileClone
	};
	static const MPT_INTERFACE_VPTR(iterator) fileIter = {
		fileGet,
		fileAdvance,
		fileReset
	};
	MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(iterator) *it;
	struct _iter_fdata *data;
	FILE *file;
	
	if (!(file = fdopen(fd, "r"))) {
		return 0;
	}
	if (!(mt = malloc(sizeof(*mt) + sizeof(*it) + sizeof(*data)))) {
		return 0;
	}
	mt->_vptr = &fileMeta;
	it = (void *) (mt + 1);
	it->_vptr = &fileIter;
	data = (void *) (it + 1);
	data->fd = file;
	data->type = 0;
	data->count = 0;
	
	return mt;
}

