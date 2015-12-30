/*!
 * loop trough object/metatype properties
 */

#include "convert.h"

struct outData
{
	MPT_TYPE(PropertyHandler) h;
	void *p;
};

static int mprint(void *data, const MPT_STRUCT(property) *prop)
{
	char buf[1024];
	struct outData *par = data;
	MPT_STRUCT(property) pr = *prop;
	int pos = 0, adv = 0;
	
	*buf = 0;
	
	if (!pr.val.fmt) {
		if (!pr.val.ptr) { pr.val.ptr = buf; *buf = 0; }
		return par->h(par->p, &pr);
	}
	if (!pr.val.ptr) {
		return *pr.val.fmt ? -1 : 0;
	}
	while (*pr.val.fmt && (adv = mpt_data_print(buf+pos, sizeof(buf)-pos, *pr.val.fmt, pr.val.ptr)) >= 0) {
		int len;
		if ((len = mpt_valsize(*pr.val.fmt)) < 0) {
			return par->h(par->p, prop);
		}
		pos += adv;
		pr.val.ptr = ((uint8_t *) pr.val.ptr) + len;
		if (*(++pr.val.fmt) && pos < (int) (sizeof(buf)-2)) {
			buf[pos++] = ' '; buf[pos] = 0;
		}
	}
	if (adv < 0 || (pos + adv) >= (int) sizeof(buf)) {
		return par->h(par->p, prop);
	}
	pr.val.ptr = buf;
	pr.val.fmt = 0;
	
	return par->h(par->p, &pr);
}

extern int mpt_generic_print(int (*get)(void *, MPT_STRUCT(property) *), void *obj, MPT_TYPE(PropertyHandler) proc, void *data, int mask)
{
	struct outData par;
	
	par.h = proc;
	par.p = data;
	
	return mpt_generic_foreach(get, obj, mprint, &par, mask);
}
