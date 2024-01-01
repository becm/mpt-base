#include <stdio.h>
#include <string.h>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(convert.h)
#include MPT_INCLUDE(types.h)

#include MPT_INCLUDE(object.h)

#include MPT_INCLUDE(layout.h)

static int printm(void *out, const struct mpt_property *prop)
{
	const void *ptr = prop->val._addr;
	const char *str = mpt_data_tostring(&ptr, prop->val._type, 0);
	/* format string check fails for print macros (PRIuPTR) */
	if (!str) return fprintf(out, "%s: %lu> %p\n", prop->name, prop->val._type, prop->val._addr);
	return fprintf(out, "%s = %s;\n", prop->name, str);
}
static int getter(void *addr, struct mpt_property *pr)
{
	return mpt_axis_get(addr, pr);
}

static int convert(MPT_INTERFACE(convertable) *src, mpt_type_t type, void *dest)
{
	return mpt_convert_string(*((char **) (src + 1)), type, dest);
}
static const MPT_INTERFACE_VPTR(convertable) src_vptr = { convert };

int main(int argc, char *argv[])
{
	MPT_STRUCT(axis) obj;
	struct {
		MPT_INTERFACE(convertable) src;
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
	
	mpt_properties_print(getter, &obj, printm, stdout, -1);
	
	mpt_axis_fini(&obj);
	
	return 0;
}

