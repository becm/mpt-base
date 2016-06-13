/*!
 * initialize and control identifier data.
 */

#include <string.h>
#include <stdlib.h>

#include "core.h"

#define MPT_IdentifierPrintable  0x1
#define MPT_IdentifierPointer    0x2

#define MPT_IDENT_SMAX 0x7
#define MPT_IDENT_PMAX 0x1fu
#define MPT_IDENT_LMAX(s) ((0x10u<<s) + (0x1fu<<(s-1)))
#define MPT_IDENT_MAX MPT_IDENT_LMAX(MPT_IDENT_SMAX)
#define MPT_IDENT_HSZE 4
#define MPT_IDENT_BSZE (sizeof(MPT_STRUCT(identifier)) - MPT_IDENT_HSZE)

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
	uint8_t shift = 0;
	
	if ((len < MPT_IDENT_BSZE) || (len > (MPT_IDENT_BSZE + MPT_IDENT_MAX))) {
		return sizeof(MPT_STRUCT(identifier));
	}
	/* no exponent required */
	if (len <= (MPT_IDENT_BSZE + MPT_IDENT_PMAX)) {
		return MPT_IDENT_HSZE + len;
	}
	len -= MPT_IDENT_BSZE;
	
	/* align size to header+data */
	while (++shift <= MPT_IDENT_SMAX) {
		size_t min, step, add;
		if (len > MPT_IDENT_LMAX(shift)) {
			continue;
		}
		step = 0x1 << (shift - 1);
		min  = 0x10 << shift;
		add  = (len - min) / step;
		
		/* smallest size above needed */
		if (len > (min += add * step)) {
			min += step;
		}
		return sizeof(MPT_STRUCT(identifier)) + min;
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
	uint8_t shift = 1;
	
	/* length out of range */
	if (len < sizeof(*id)) {
		len = 0;
	} else {
		len -= sizeof(*id);
	}
	/* initial header values */
	id->_len = 0;
	id->_flags = 0;
	
	if (len <= MPT_IDENT_PMAX) {
		id->_post = len;
		len += MPT_IDENT_BSZE;
		memset(id->_val, 0, len);
		return;
	}
	/* find matching exponent */
	else while (shift <= MPT_IDENT_SMAX) {
		if (len <= MPT_IDENT_LMAX(shift)) {
			size_t min, step, add;
			
			step = 0x1 << (shift - 1);
			min  = 0x10 << shift;
			add  = (len - min) / step;
			
			/* smallest size above needed */
			if (len > (min += add * step)) {
				++add;
				min += step;
			}
			id->_post = (shift << 0x5) + add;
			len = min + MPT_IDENT_BSZE;
			
			memset(id->_val, 0, len);
			return;
		}
		++shift;
	}
	/* max size */
	id->_post = 0xff;
	len = MPT_IDENT_BSZE + MPT_IDENT_MAX;
	
	/* clear covered data */
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
extern const void *mpt_identifier_copy(MPT_STRUCT(identifier) *id, const MPT_STRUCT(identifier) *from)
{
	const void *base;
	if (!from) {
		return mpt_identifier_set(id, 0, 0);
	}
	base = (from->_flags & MPT_IdentifierPointer) ? from->_base : from->_val;
	
	if (!(base = mpt_identifier_set(id, base, -from->_len))) {
		return 0;
	}
	id->_flags = (id->_flags ^ MPT_IdentifierPrintable) | (from->_flags | MPT_IdentifierPrintable);
	
	return base;
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
extern const void *mpt_identifier_set(MPT_STRUCT(identifier) *id, const char *name, int nlen)
{
	char *addr = id->_val;
	int len;
	uint8_t max;
	
	/* auto-detect length */
	len = (nlen > 0) ? nlen + 1 : -nlen;
	
	/* max length exceeded */
	if (len > UINT16_MAX) {
		return 0;
	}
	/* get size of ident data */
	if ((max = id->_post) >= (0x1<<0x5)) {
		uint8_t shift = max >> 0x5;
		max = (0x10 << shift) + ((max & 0x1f) << (shift-1));
	}
	max += MPT_IDENT_BSZE;
	
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
	if (!nlen) {
		if (name) {
			id->_flags |= MPT_IdentifierPrintable;
			addr[0] = 0;
		} else {
			id->_flags &= ~MPT_IdentifierPrintable;
		}
		return memset(id->_val, 0, MPT_IDENT_BSZE);
	}
	else if (nlen < 0) {
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
 * \brief compare identifier
 * 
 * Check if identifier are equal.
 * 
 * \param id   first identifier
 * \param cmp  second identifier
 * 
 * \return start of identifier data
 */
extern int mpt_identifier_inequal(const MPT_STRUCT(identifier) *id, const MPT_STRUCT(identifier) *cmp)
{
	const char *idbase;
	const char *cmpbase;
	
	int diff;
	
	if ((diff = id->_len - cmp->_len)) {
		return diff;
	}
	idbase = id->_val;
	if (id->_flags & MPT_IdentifierPointer) {
		idbase = id->_base;
	}
	cmpbase = cmp->_val;
	if (cmp->_flags & MPT_IdentifierPointer) {
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
	return (id->_flags & MPT_IdentifierPointer) ? id->_base : id->_val;
}
/*!
 * \ingroup mptCore
 * \brief identifier printable length
 * 
 * return printable identifier size.
 * 
 * \param id  address of identifier
 * 
 * \return length of printable data (negative for non-printable)
 */
extern int mpt_identifier_len(const MPT_STRUCT(identifier) *id)
{
	return (id->_flags & MPT_IdentifierPrintable) ? (id->_len ? id->_len - 1 : 0) : -id->_len;
}
