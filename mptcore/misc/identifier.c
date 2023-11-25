/*!
 * initialize and control identifier data.
 */

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "types.h"

#include "core.h"

#define MPT_IDENT_HSZE 4
#define MPT_IDENT_VSZE (MPT_IDENT_HSZE + 4 + sizeof(void *))
#define MPT_IDENT_MAX  (0x100 - MPT_IDENT_HSZE)
#define MPT_IDENT_BSZE (sizeof(MPT_STRUCT(identifier)) - MPT_IDENT_HSZE)


static void _identifier_fini(void *ptr)
{
	MPT_STRUCT(identifier) *ident = ptr;
	if (ident->_len > ident->_max) {
		free(ident->_base);
		memset(ident->_val, 0, ident->_max);
	}
}
static int _identifier_init(void *ptr, const void *src)
{
	MPT_STRUCT(identifier) *c = ptr;
	mpt_identifier_init(c, sizeof(*c));
	if (src && mpt_identifier_copy(c, src)) {
		return MPT_ERROR(BadOperation);
	}
	return c ? 1 : 0;
}

/*!
 * \ingroup mptCore
 * \brief get identifier traits
 * 
 * Get identifier operations and size.
 * 
 * \return identifier type traits
 */
extern const MPT_STRUCT(type_traits) *mpt_identifier_traits()
{
	static const MPT_STRUCT(type_traits) traits = {
		_identifier_init,
		_identifier_fini,
		sizeof(MPT_STRUCT(identifier))
	};
	return &traits;
}

/*!
 * \ingroup mptCore
 * \brief identifier size
 * 
 * Align identifier size for requested lenth
 * 
 * \param len  requested length for identifier
 * 
 * \return needed size for identifier
 */
MPT_STRUCT(identifier) *mpt_identifier_new(size_t len)
{
	MPT_STRUCT(identifier) *id;
	size_t size = 4 * sizeof(void *);
	
	/* bad length constraint */
	if (len > UINT16_MAX) {
		errno = EINVAL;
		return 0;
	}
	/* non-default identifier size */
	len += MPT_IDENT_HSZE;
	if (len > size && len <= 0x100) {
		while (size < len) {
			size *= 2; /* (16,) 32, 64, 128, 256 */
		}
	}
	if (!(id = malloc(size))) {
		return 0;
	}
	mpt_identifier_init(id, size);
	return id;
}

/*!
 * \ingroup mptCore
 * \brief initialize identifier
 * 
 * Set size of usable data for identifier.
 * 
 * \param id   address of identifier header
 * \param len  length of memory (including header)
 */
extern void mpt_identifier_init(MPT_STRUCT(identifier) *id, size_t len)
{
	/* length out of range */
	if (len < MPT_IDENT_HSZE) {
		return;
	}
	if ((len -= MPT_IDENT_HSZE) > MPT_IDENT_MAX) {
		len = MPT_IDENT_MAX;
	}
	/* initial header values */
	id->_len = 0;
	id->_charset = 0;
	id->_max = len;
	memset(id->_val, 0, len);
}


/*!
 * \ingroup mptCore
 * \brief copy identifier
 * 
 * Copy identifier data.
 * 
 * \param id    address of identifier header
 * \param from  source identifier
 */
extern void *mpt_identifier_copy(MPT_STRUCT(identifier) *id, const MPT_STRUCT(identifier) *from)
{
	const void *base;
	void *dest;
	
	if (!from) {
		return mpt_identifier_set(id, 0, 0);
	}
	if (id == from) {
		return (id->_len > id->_max) ? id->_base : id->_val;
	}
	base = (from->_len > from->_max) ? from->_base : from->_val;
	if (from->_len <= id->_max) {
		dest = id->_val;
	}
	else if (!(dest = malloc(from->_len))) {
		return 0;
	}
	memcpy(dest, base, from->_len);
	if (id->_len > id->_max) {
		free(id->_base);
		id->_base = 0;
	}
	if (dest != id->_val) {
		memset(id->_val, 0, sizeof(id->_val));
		id->_base = dest;
	}
	id->_len  = from->_len;
	id->_charset = from->_charset;
	
	return dest;
}

/*!
 * \ingroup mptCore
 * \brief assign identifier
 * 
 * Set identifier data to specified value.
 * Pass zero pointer for base address to indicate non-printable data.
 * 
 * \param id    address of identifier header
 * \param name  start of name
 * \param len   length of name
 * 
 * \return start of identifier data
 */
extern void *mpt_identifier_set(MPT_STRUCT(identifier) *id, const char *name, int len)
{
	char *dest, *addr;
	int nlen = len;
	int charset = 0;
	
	/* set name data */
	if (name) {
		if (len < 0) {
			len = strlen(name);
		}
		/* check _charset for base set */
		charset = MPT_CHARSET(UTF8);
		nlen = len + 1;
	}
	/* max length exceeded */
	if (len < 0 || nlen > UINT16_MAX) {
		return 0;
	}
	/* length exceeds reserved size */
	if (nlen > id->_max) {
		if (!(addr = malloc(nlen))) {
			return 0;
		}
		/* copy name content */
		if (name) {
			memcpy(addr, name, len);
			if (nlen > len) {
				addr[len] = 0;
			}
		} else {
			memset(addr, 0, nlen);
		}
		/* clear old allocation */
		if ((id->_len > id->_max) && id->_base) {
			free(id->_base);
		}
		/* set new name content address */
		memset(id->_val, 0, id->_max);
		id->_len = nlen;
		id->_base = addr;
		
		id->_charset = charset;
		
		return addr;
	}
	/* local data sufficient */
	addr = (id->_len > id->_max) ? id->_base : 0;
	if (len) {
		int post = id->_max - len;
		dest = memcpy(id->_val, name, len);
		if (post) {
			memset(id->_val + len, 0, post);
		}
	} else {
		dest = memset(id->_val, 0, id->_max);
	}
	/* clear potential old allocation */
	if (addr) {
		free(addr);
	}
	id->_len  = nlen;
	id->_charset = charset;
	
	return dest;
}

/*!
 * \ingroup mptCore
 * \brief compare identifier
 * 
 * Check if identifier data matches.
 * 
 * \param id    address of identifier
 * \param name  data to compare
 * \param nlen  data length
 * 
 * \retval 0  identifier matches
 * \retval >0 different data content
 * \retval mpt::BadArgument  invalid argument combination
 * \retval mpt::BadType      identifier has non-character content
 * \retval mpt::MissingData  lengt differs
 */
extern int mpt_identifier_compare(const MPT_STRUCT(identifier) *id, const char *name, int nlen)
{
	const char *base = id->_val;
	int i;
	
	if (name && (id->_charset != MPT_CHARSET(UTF8))) {
		return MPT_ERROR(BadType);
	}
	if (nlen < 0) {
		if (!name) {
			return MPT_ERROR(BadArgument);
		}
		nlen = strlen(name);
	}
	if (!nlen && !id->_len) {
		return 0;
	}
	if ((nlen + 1) != id->_len) {
		return MPT_ERROR(MissingData);
	}
	if (id->_len > id->_max) {
		base = id->_base;
	}
	if (!name) {
		for (i = 0; i <= nlen; ++i) {
			if (base[i]) {
				return i + 1;
			}
		}
		return 0;
	}
	for (i = 0; i < nlen; ++i) {
		if (base[i] != name[i]) {
			return i + 1;
		}
	}
	if (base[nlen]) {
		return nlen;
	}
	return 0;
}

/*!
 * \ingroup mptCore
 * \brief compare identifier
 * 
 * Check if identifier are of same type and value.
 * 
 * \param id   first identifier
 * \param cmp  second identifier
 * 
 * \return zero on equality
 */
extern int mpt_identifier_inequal(const MPT_STRUCT(identifier) *id, const MPT_STRUCT(identifier) *cmp)
{
	const char *idbase;
	const char *cmpbase;
	int diff;
	
	if ((diff = id->_charset - cmp->_charset)) {
		return diff;
	}
	if ((diff = id->_len - cmp->_len)) {
		return diff;
	}
	idbase = id->_val;
	if (id->_len > id->_max) {
		idbase = id->_base;
	}
	cmpbase = cmp->_val;
	if (cmp->_len > cmp->_max) {
		cmpbase = cmp->_base;
	}
	return memcmp(idbase, cmpbase, id->_len);
}
/*!
 * \ingroup mptCore
 * \brief identifier value
 * 
 * Get identifier data matches.
 * 
 * \param id  address of identifier
 * 
 * \return start of identifier data
 */
extern const void *mpt_identifier_data(const MPT_STRUCT(identifier) *id)
{
	return (id->_len > id->_max) ? id->_base : id->_val;
}
