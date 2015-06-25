#include <stdio.h>
#include <string.h>

#include <mpt/config.h>
#include <mpt/plot.h>
#include <mpt/convert.h>

static int printm(void *out, struct mpt_property *prop)
{
	if (prop->val.fmt) return fprintf(out, "%s: <%s> %p\n", prop->name, prop->val.fmt, prop->val.ptr);
	return fprintf(out, "%s = %s;\n", prop->name, (char *) prop->val.ptr);
}

static int getter(void *addr, struct mpt_property *pr)
{ return mpt_axis_pget(addr, pr, 0); }

static int convert(MPT_INTERFACE(source) *src, int type, void *dest)
{
	return mpt_convert_string((void *) (src+1), type, dest);
}

static MPT_INTERFACE_VPTR(source) src_vptr = { convert };

int main(int argc, char *argv[])
{
	struct mpt_axis obj;
	struct mpt_property pr;
	struct {
		MPT_INTERFACE(source) src;
		char *txt;
	} src;
	
	src.src._vptr = &src_vptr;
	
	mpt_axis_init(&obj, 0);
	
	++argv;
	while (*argv) {
		src.txt = strchr(pr.name = *argv, '=');
		++argv;
		if (!src.txt) continue;
		*(src.txt++) = 0;
		if (mpt_axis_pget(&obj, &pr, &src.src) < 0) {
			perror(pr.name);
		}
	}
	
	mpt_generic_print(getter, &obj, printm, stdout, 0);
	
	return 0;
}

