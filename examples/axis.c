#include <stdio.h>
#include <string.h>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(convert.h)
#include MPT_INCLUDE(config.h)
#include MPT_INCLUDE(meta.h)

#include MPT_INCLUDE(layout.h)

static int printm(void *out, const struct mpt_property *prop)
{
	if (prop->val.fmt) return fprintf(out, "%s: <%s> %p\n", prop->name, prop->val.fmt, prop->val.ptr);
	return fprintf(out, "%s = %s;\n", prop->name, (char *) prop->val.ptr);
}
static int getter(void *addr, struct mpt_property *pr)
{
	return mpt_axis_get(addr, pr);
}

static void unref(MPT_INTERFACE(reference) *src)
{
	(void) src;
}
static uintptr_t addref(MPT_INTERFACE(reference) *src)
{
	(void) src;
	return 0;
}
static int convert(const MPT_INTERFACE(metatype) *src, int type, void *dest)
{
	return mpt_convert_string(*((char **) (src+1)), type, dest);
}
static MPT_INTERFACE(metatype) *clone(const MPT_INTERFACE(metatype) *src)
{
	(void) src; return 0;
}
static MPT_INTERFACE_VPTR(metatype) src_vptr = { { unref, addref }, convert, clone };

int main(int argc, char *argv[])
{
	MPT_STRUCT(axis) obj;
	struct {
		MPT_INTERFACE(metatype) src;
		char *txt;
	} src;
	
	src.src._vptr = &src_vptr;
	
	mpt_axis_init(&obj, 0);
	
	++argv;
	while (--argc) {
		const char *name;
		src.txt = strchr(name = *argv, '=');
		++argv;
		if (!src.txt) continue;
		*(src.txt++) = 0;
		if (mpt_axis_set(&obj, name, &src.src) < 0) {
			perror(name);
		}
	}
	
	mpt_generic_print(getter, &obj, printm, stdout, -1);
	
	return 0;
}

