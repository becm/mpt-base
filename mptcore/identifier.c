/*!
 * initialize and control identifier data.
 */

#include <string.h>
#include <stdlib.h>

#include "core.h"

#define MPT_IdentifierPrintable  0x1
#define MPT_IdentifierPointer    0x2

#define MPT_IDENT_SMAX 0x7
#define MPT_IDENT_VMAX 0x1f
#define MPT_IDENT_LMAX(s) ((0x10u<<s) + (0x1fu<<(s-1)))
#define MPT_IDENT_MAX MPT_IDENT_LMAX(MPT_IDENT_SMAX)
#define MPT_IDENT_HSZE 4

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
size_t mpt_identifier_align(size_t len)
{
	size_t rem = MPT_IDENT_VMAX;
	uint8_t shift = 0, min, max;
	
	if (len > UINT16_MAX) {
		return sizeof(MPT_STRUCT(identifier));
	}
	/* no exponent required */
	if (rem >= len) {
		rem = len + MPT_IDENT_HSZE;
		return rem < sizeof(MPT_STRUCT(identifier)) ? sizeof(MPT_STRUCT(identifier)) : rem;
	}
	/* align size to header+data */
	while (++shift < MPT_IDENT_SMAX) {
		rem = MPT_IDENT_LMAX(shift);
		if (rem < len) {
			continue;
		}
		max = shift + 4;
		min = shift - 1;
		
		/* reduce maximal length for current exponent */
		while (max > min) {
			size_t tmp = rem - (0x1 << (--max));
			if (tmp >= len) {
				rem = tmp;
			}
		}
		return rem + MPT_IDENT_HSZE;
	}
	/* length exceeds local size */
	return sizeof(MPT_STRUCT(identifier));
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
	size_t rem;
	uint8_t shift = 1;
	
	/* length out of range */
	if (len < sizeof(*id) || (len -= sizeof(*id)) > MPT_IDENT_MAX) {
		len = 0;
	}
	rem = len + sizeof(id->_val) + sizeof(id->_base);
	
	/* initial header values */
	id->_len = 0;
	id->_size = 0;
	id->_flags = 0;
	
	if (len <= MPT_IDENT_VMAX) {
		id->_size = rem;
	}
	/* find matching exponent */
	else while (shift <= MPT_IDENT_SMAX) {
		if (rem <= MPT_IDENT_LMAX(shift)) {
			id->_size = shift << 0x5;
			rem = (rem - (0x10 << shift)) >> (shift - 1);
			break;
		}
		++shift;
	}
	/* save 'mantissa' */
	id->_size |= rem;
	
	/* empty data */
	memset(id->_val, 0, len + sizeof(id->_val));
}

/*!
 * \ingroup mptCore
 * \brief initialize identifier
 * 
 * Set identifier data tp specified value.
 * Pass zero pointer for base address to indicate non-printable data.
 * 
 * \param id    address of identifier header
 * \param name  start of name
 * \param len   length of name
 * 
 * \return start of identifier data
 */
extern void *mpt_identifier_set(MPT_STRUCT(identifier) *id, const char *name, int nlen)
{
	int len;
	char *addr = id->_val;
	uint8_t max;
	
	/* auto-detect length */
	len = (nlen > 0) ? nlen + 1 : -nlen;
	
	/* max length exceeded */
	if (len > UINT16_MAX) {
		return 0;
	}
	/* get size of ident data */
	if ((max = id->_size) >= (0x1<<0x5)) {
		uint8_t shift = max >> 0x5;
		max = (0x10 << shift) + ((max & 0x1f) << (shift-1));
	}
	/* length exceeds local size */
	if (len > max) {
		addr = (id->_flags & MPT_IdentifierPointer) ? id->_base : 0;
		
		if (!(addr = realloc(addr, len))) {
			return 0;
		}
		id->_base = addr;
		id->_flags |= MPT_IdentifierPointer;
	}
	/* data fits local data */
	else if (id->_flags & MPT_IdentifierPointer) {
		free(id->_base);
	}
	id->_len = len;
	
	/* set identifier type */
	if (nlen < 0) {
		id->_flags &= ~MPT_IdentifierPrintable;
		nlen = -nlen;
	}
	else {
		id->_flags |= MPT_IdentifierPrintable;
		addr[nlen] = 0;
	}
	if (name) {
		return memcpy(addr, name, nlen);
	}
	return memset(addr, 0, nlen);
}

/*!
 * \ingroup mptCore
 * \brief compare identifier
 * 
 * Check if identifier data matches.
 * 
 * \param id    address of identifier
 * \param name  data to compare
 * \param nlen  data length (<0 for non-printable)
 * 
 * \return start of identifier data
 */
extern int mpt_identifier_compare(const MPT_STRUCT(identifier) *id, const char *name, int nlen)
{
	const char *base = id->_val;
	
	if (id->_flags & MPT_IdentifierPointer) {
		base = id->_base;
	}
	if (nlen < 0) {
		if (!name) {
			return -2;
		}
		if ((nlen = -nlen) != id->_len) {
			return nlen - id->_len;
		}
	}
	else {
		if (!nlen && !id->_len) {
			return 0;
		}
		if ((nlen + 1) != id->_len) {
			return -2;
		}
		if (base[nlen]) {
			return nlen;
		}
	}
	return memcmp(base, name, nlen);
}

/*!
 * \ingroup mptCore
 * \brief identifier value
 * 
 * Get identifier data matches.
 * 
 * \param id   address of identifier
 * \param len  data length (required for non-printable)
 * 
 * \return start of identifier data
 */
extern const void *mpt_identifier_data(const MPT_STRUCT(identifier) *id, size_t *len)
{
	if (len) {
		*len = id->_len;
	}
	else if (!(id->_flags & MPT_IdentifierPrintable)) {
		return 0;
	}
	return id->_flags & MPT_IdentifierPointer ? id->_base : id->_val;
}
