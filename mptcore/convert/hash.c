/*!
 * set and execute default MPT hash function.
 */

#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <stdint.h>

#include "convert.h"

static uintptr_t (*hashFcn)(const void *, size_t );

extern int _mpt_hash_set(const char *type)
{
	if (hashFcn) {
		return -3;
	}
	if (!type || !*type || strcasecmp(type, "djb2")) {
		hashFcn = mpt_hash_djb2;
		return 0;
	}
	if (!strcasecmp(type, "smdb")) {
		hashFcn = mpt_hash_smdb;
		return 1;
	}
	return -2;
}
extern uintptr_t mpt_hash(const void *str, size_t len)
{
	if (!hashFcn) hashFcn = mpt_hash_djb2;
	return hashFcn(str, len);
}
