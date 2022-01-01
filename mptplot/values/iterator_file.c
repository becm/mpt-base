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
#include "types.h"

#include "values.h"

MPT_STRUCT(iteratorFile)
{
	MPT_INTERFACE(metatype) _mt;
	MPT_INTERFACE(iterator) _it;
	MPT_INTERFACE(convertable) _conv;
	MPT_STRUCT(value) value;
	
	FILE *fd;
	int type;
	int count;
	uint8_t val[16];
};

/* convertable interface */
static int fileConv(MPT_INTERFACE(convertable) *val, int type, void *ptr)
{
	MPT_STRUCT(iteratorFile) *d = MPT_baseaddr(iteratorFile, val, _mt);
	
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeIteratorPtr), MPT_ENUM(TypeFilePtr) };
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
			return 0;
		}
		return MPT_ENUM(TypeIteratorPtr);
	}
	if (type == MPT_ENUM(TypeIteratorPtr)) {
		if (ptr) *((const void **) ptr) = &d->_it;
		return MPT_ENUM(TypeFilePtr);
	}
	if (type == MPT_ENUM(TypeFilePtr)) {
		if (ptr) *((void **) ptr) = d->fd;
		return MPT_ENUM(TypeIteratorPtr);
	}
	return MPT_ERROR(BadType);
}
/* metatype interface */
static void fileUnref(MPT_INTERFACE(metatype) *mt)
{
	MPT_STRUCT(iteratorFile) *d = MPT_baseaddr(iteratorFile, mt, _mt);
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
/* element convertable interface */
static int fileGet(MPT_INTERFACE(convertable) *conv, int type, void *ptr)
{
	MPT_STRUCT(iteratorFile) *d = MPT_baseaddr(iteratorFile, conv, _conv);
	MPT_TYPE(data_converter) converter;
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
		return MPT_ENUM(TypeFilePtr);
	}
	else if (d->type < 0) {
		return 0;
	}
	if (!(converter = mpt_data_converter(d->type))) {
		return MPT_ERROR(BadType);
	}
	return (ret = converter(d->val, type, ptr)) < 0 ? ret : d->type;
}
/* iterator interface */
static const MPT_STRUCT(value) *fileValue(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(iteratorFile) *d = MPT_baseaddr(iteratorFile, it, _it);
	return &d->value;
}
static int fileAdvance(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(iteratorFile) *d = MPT_baseaddr(iteratorFile, it, _it);
	if (d->type < 0) {
		return MPT_ERROR(MissingData);
	}
	if (!d->type) {
		int ret;
		if ((ret = fileGet(&d->_conv, 'd', 0)) < 0) {
			return ret;
		}
		++d->count;
		return 0;
	}
	d->type = 0;
	return MPT_ENUM(TypeFilePtr);
}
static int fileReset(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(iteratorFile) *d = MPT_baseaddr(iteratorFile, it, _it);
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
		fileValue,
		fileAdvance,
		fileReset
	};
	static const MPT_INTERFACE_VPTR(convertable) fileElem = {
		fileGet
	};
	MPT_STRUCT(iteratorFile) *data;
	MPT_INTERFACE(convertable) *conv;
	FILE *file;
	
	if (!(file = fdopen(fd, "r"))) {
		return 0;
	}
	if (!(data = malloc(sizeof(*data)))) {
		return 0;
	}
	data->_mt._vptr = &fileMeta;
	data->_it._vptr = &fileIter;
	data->_conv._vptr = &fileElem;
	
	conv = &data->_conv;
	MPT_value_set_data(&data->value, MPT_ENUM(TypeConvertablePtr), &conv);
	
	data->fd = file;
	data->type = 0;
	data->count = 0;
	
	return &data->_mt;
}

