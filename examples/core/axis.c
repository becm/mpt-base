#include <stdio.h>
#include <string.h>

#include <mpt/config.h>
#include <mpt/layout.h>
#include <mpt/convert.h>

int printm(void *out, struct mpt_property *prop)
{
	if (prop->fmt) return fprintf(out, "org <%s> %p\n", prop->fmt, prop->data);
	return fprintf(out, "%s = %s;\n", prop->name, (char *) prop->data);
}

int getter(void *addr, struct mpt_property *pr)
{ return mpt_axis_pget(addr, pr, 0); }

int convert(MPT_INTERFACE(source) *src, int type, void *dest)
{
	const char **txt = (void *) (src+1);
	int len = mpt_convert(*txt, type, dest);
	if (len > 0 && dest) *txt += len;
	return len;
}

static MPT_INTERFACE_VPTR(source) src_vptr = { convert };

int main(int argc, char *argv[])
{
	struct mpt_axis	obj;
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

