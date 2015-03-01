/*!
 * loop trough object/metatype properties
 */

#include "convert.h"

struct outData
{
	MPT_TYPE(PropertyHandler) h;
	void *p;
};

static int mprint(void *data, MPT_STRUCT(property) *prop)
{
	char buf[1024];
	struct outData *par = data;
	MPT_STRUCT(property) pr = *prop;
	const char *fmt;
	const void *base;
	int pos = 0, adv = 0;
	
	*buf = 0;
	
	if (!(fmt = pr.fmt)) {
		if (!pr.data) { pr.data = buf; *buf = 0; }
		return par->h(par->p, &pr);
	}
	if (!(base = pr.data)) {
		return *fmt ? -1 : 0;
	}
	while (*fmt && (adv = mpt_data_print(buf+pos, sizeof(buf)-pos, *pr.fmt, &base)) >= 0) {
		pos += adv;
		if (*(++fmt) && pos < (int) (sizeof(buf)-2)) {
			buf[pos++] = ' '; buf[pos] = 0;
		}
		adv = 0;
	}
	if (adv < 0 || (pos + adv) >= (int) sizeof(buf)) {
		return par->h(par->p, &pr);
	}
	pr.data = buf;
	pr.fmt  = 0;
	
	return par->h(par->p, &pr);
}

extern int mpt_generic_print(MPT_TYPE(PropertyHandler) get, void *obj, MPT_TYPE(PropertyHandler) proc, void *data, int mask)
{
	struct outData par;
	
	par.h = proc;
	par.p = data;
	
	return mpt_generic_foreach(get, obj, mprint, &par, mask);
}
