/*!
 * set metatype data.
 */

#include <ctype.h>
#include <string.h>

#include <sys/uio.h>

#include "convert.h"
#include "core.h"


struct propParam {
	const char *fmt;
	const void *data;
	const char *sep;
	int (*conv)(const char **, int , void *);
};

#ifndef MPT_NO_CONVERT
static int fromText(struct propParam *pr, int type, void *dest)
{
	const char *txt[2];
	int	len;
	
	if (*pr->fmt == 's') {
		txt[0] = *(void **) pr->data;
		if (type == 's') {
			*(const void **) dest = txt[0];
			return txt[0] ? strlen(txt[0]) : 0;
		}
		if (!txt[0]) return -2;
	} else if (*pr->fmt == ((char) (MPT_ENUM(TypeVector) | 'c'))) {
		const struct iovec *vec = pr->data;
		if (type == *pr->fmt || type == MPT_ENUM(TypeVector)) {
			*((struct iovec *) dest) = *vec;
			return sizeof(*vec);
		}
		if (type == 's') {
			*(void **) dest = vec->iov_base;
			return vec->iov_len;
		}
		if (!(txt[0] = vec->iov_base) || !memchr(txt[0], 0, vec->iov_len)) return -2;
	} else {
		return -3;
	}
	
	txt[1] = pr->sep;
	if ((len = pr->conv(txt, type, dest)) <= 0 || !dest) return len;
	
	pr->data = txt[0];
	pr->fmt  = 0;
	
	return len;
}
#endif

static int propConv(MPT_INTERFACE(source) *ctl, int type, void *dest)
{
	struct propParam *src = (void *) (ctl + 1);
	const char *txt[2];
	int	len;
	
	if (src->fmt) {
		const void *from = src->data;
#ifdef MPT_NO_CONVERT
		if (!src->fmt[0] || type != src->fmt[0]) {
			return -1;
		}
		if ((len = mpt_valsize(type)) <= 0) {
			return -3;
		}
		if (!dest) return len;
		if (!len) len = sizeof(void *);
		
		memcpy(dest, from, len);
		from = ((uint8_t *) from) + len;
#else
		if ((len = mpt_data_convert(&from, *src->fmt, dest, type)) < 0) {
			return fromText(src, type, dest);
		}
#endif
		if (dest) src->data = from;
		++src->fmt;
		return len;
	}
	if (!(txt[0] = src->data)) {
		if (type != 'k' && type != 's') {
			return -3;
		}
		if (dest) ((char **) dest)[0] = 0;
		return 0;
	}
	txt[1] = src->sep;
	if ((len = src->conv(txt, type, dest)) <= 0 || !dest) {
		return len;
	}
	src->data = txt[0];
	return len;
}

static const MPT_INTERFACE_VPTR(source) _prop_vptr = {
	propConv
};

static int stringConvert(const char **from, int type, void *dest)
{
#ifdef MPT_NO_CONVERT
	(void) from;
	(void) type;
	(void) dest;
	return -1;
#else
	if (type == 'k') {
		const char *txt;
		size_t	klen;
		if (!(txt = mpt_convert_key(from, from[1], &klen))) return -2;
		if (dest) ((const char **) dest)[0] = txt;
		return klen;
	}
	return mpt_convert_string(from, type, dest);
#endif
}


/*!
 * \ingroup mptMeta
 * \brief set solver property
 * 
 * Set solver parameter.
 * 
 * \param gen	solver descriptor
 * \param par	parameter to change
 * \param fmt	data format
 * \param data	address of data to set
 */
extern int mpt_meta_pset(MPT_INTERFACE(metatype) *meta, MPT_INTERFACE(property) *pr, int (*conv)(const char **, int ,void *))
{
	struct {
		MPT_INTERFACE(source) ctl;
		struct propParam d;
	} src;
	
	src.ctl._vptr = &_prop_vptr;
	src.d.fmt = pr->fmt;
	src.d.data = pr->data;
	src.d.sep = pr->desc ? pr->desc : " ,;/:";
	src.d.conv = conv ? conv : stringConvert;
	
	return meta->_vptr->property(meta, pr, &src.ctl);
}

/*!
 * \ingroup mptMeta
 * \brief set solver property
 * 
 * Set solver parameter.
 * 
 * \param gen	solver descriptor
 * \param par	parameter to change
 * \param fmt	data format
 * \param data	address of data to set
 */
extern int mpt_meta_set(MPT_INTERFACE(metatype) *m, const char *par, const char *fmt, const void *data)
{
	MPT_STRUCT(property) prop;
	
	prop.name = par ? par : "";
	prop.desc = 0;
	prop.fmt  = fmt;
	prop.data = data;
	
	return mpt_meta_pset(m, &prop, 0);
}
