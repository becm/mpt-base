/*!
 * calculate hash for data.
 * 
 * using modified djb2-algorythm:
 *	hash(i) = hash(i - 1) * 33 ^ str[i]
 * original djb2-algorythm
 *	hash(i) = hash(i - 1) * 33 + str[i]
 */

#include "core.h"

extern uintptr_t mpt_hash_djb2(const void *data, int len)
{
	const char *str;
	uintptr_t hash = 5381;
	
	if (!(str = data)) {
		return 0;
	}
	if (len < 0) {
		while (*str) {
			hash = (hash * 33) ^ *str++;
		}
		return hash;
	}
	while (len--) {
		hash = (hash * 33) ^ *str++;
	}
	return hash;
}
