/*!
 * calculate hash for data.
 * 
 * using modified djb2-algorythm:
 *	hash(i) = hash(i - 1) * 33 ^ str[i]
 * original djb2-algorythm
 *	hash(i) = hash(i - 1) * 33 + str[i]
 */

#include "convert.h"

extern uintptr_t mpt_hash_djb2(const void *data, size_t len)
{
	const char *str = data;
	uintptr_t hash = 5381;
	
	if (!str) {
		return 0;
	}
	else if (len) {
		while (len--) {
			hash = hash * 33 ^ *str++;
		}
	}
	else {
		while (*str) {
			hash = hash * 33 ^ *str++;
		}
	}
	return hash;
}
