/*!
 * calculate hash for data.
 * 
 * use smdb-algorythm:
 *    hash(i) = hash(i - 1) * 65599 + str[i]
 */

#include "convert.h"

extern uintptr_t mpt_hash_smdb(const void *data, size_t len)
{
	const char *str = data;
	uintptr_t hash = 0;
	
	if (!str) {
		return 0;
	}
	else if (len) {
		while (len--) {
			hash = hash * 65599 + *str++;
		}
	}
	else {
		while (*str) {
			hash = hash * 65599 + *str++;
		}
	}
	return hash;
}

