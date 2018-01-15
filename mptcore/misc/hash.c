/*!
 * set and execute default MPT hash function.
 */

#include <string.h>
#include <strings.h>

#include <stdint.h>

#include "core.h"

static uintptr_t (*hashFcn)(const void *, int);

extern int _mpt_hash_set(const char *type)
{
	if (hashFcn) {
		return MPT_ERROR(BadOperation);
	}
	if (!type || !*type || strcasecmp(type, "djb2")) {
		hashFcn = mpt_hash_djb2;
		return 0;
	}
	if (!strcasecmp(type, "smdb")) {
		hashFcn = mpt_hash_smdb;
		return 1;
	}
	return MPT_ERROR(BadArgument);
}
extern uintptr_t mpt_hash(const void *str, int len)
{
	if (!hashFcn) {
		hashFcn = mpt_hash_djb2;
	}
	return hashFcn(str, len);
}
