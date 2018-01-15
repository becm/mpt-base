/*!
 * calculate hash for data.
 * 
 * use smdb-algorythm:
 *    hash(i) = hash(i - 1) * 65599 + str[i]
 */

#include "core.h"

extern uintptr_t mpt_hash_smdb(const void *data, int len)
{
	const char *str;
	uintptr_t hash = 0;
	
	if (!(str = data)) {
		return 0;
	}
	if (len < 0) {
		while (*str) {
			hash = (hash * 65599) + *str++;
		}
		return hash;
	}
	while (len--) {
		hash = (hash * 65599) + *str++;
	}
	return hash;
}
