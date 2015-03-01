/*!
 * control and initializer for mmap-based data.
 */

#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>

#include <sys/mman.h>

#include "array.h"

#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
# define MAP_ANONYMOUS	MAP_ANON
#endif

#if defined(MAP_ANONYMOUS)
# define MPT_MMAP_TYPE	(MAP_PRIVATE | MAP_ANONYMOUS)
#else
# include <fcntl.h>
# include <sys/stat.h>
# define MPT_MMAP_TYPE	MAP_PRIVATE
#endif

#define MPT_MMAP_FLAGS	(PROT_READ | PROT_WRITE)

static int ptsize = 0;

/*!
 * \ingroup mptArray
 * \brief memory mapped data
 * 
 * Wrapper for platform abstraction.
 * 
 * \param len	size of mmap data
 * \param base	start address hint
 * 
 * \return mapped data start address
 */
extern void *_mpt_memmap(size_t len, void *base)
{
	int	devzero = -1;
#if !defined(MAP_ANONYMOUS)
	if ((devzero = open("/dev/null", O_RDWR)) < 0)
		return 0;
#endif
	/* NULL page mapping -> memory penalty for shitty system */
	if (!(base = mmap(base, len, MPT_MMAP_FLAGS, MPT_MMAP_TYPE, devzero, 0))) {
		/* get page table size & check range */
		if (!ptsize || (ptsize = sysconf(_SC_PAGESIZE)) < 1)
			return 0;
		if (len > (size_t) ptsize) munmap(base+ptsize, len-ptsize);
		(void) mprotect(base, ptsize, PROT_NONE);
		base = mmap(base, len, MPT_MMAP_FLAGS, MPT_MMAP_TYPE, devzero, 0);
	}
#if !defined(MAP_ANONYMOUS)
	(void) close(devzero);
#endif
	/* memory mapping failed */
	return (base == MAP_FAILED) ? 0 : base;
}
/*!
 * \ingroup mptArray
 * \brief resize buffer data
 * 
 * Buffer mmap resize modifier.
 * 
 * \param b	buffer pointer
 * \param size	requested new buffer size
 * 
 * \return new buffer sattisfying size
 */
extern MPT_STRUCT(buffer) *_mpt_buffer_remap(MPT_STRUCT(buffer) *buf, size_t len)
{
	size_t	size, used;
	
	/* get page table size & check range */
	if (!ptsize || (ptsize = sysconf(_SC_PAGESIZE)) < 1)
		return 0;
	
	if (SIZE_MAX - ptsize < len) {
		errno = ERANGE; return 0;
	}
	if (buf) {
		size = buf->size;
		used = buf->used;
	} else {
		size = used = 0;
	}
	/* no change */
	if (size == len && (!buf || !buf->shared))
		return buf;
	
	/* round up to page table size */
	if (len) {
		len += sizeof(*buf);
		len = ptsize * ((len-1)/ptsize + 1);
	}
	size += sizeof(*buf);
	
	/* shrink buffer */
	if (len < size && !buf->shared) {
		if (munmap(((uint8_t *) buf)+len, size-len))
			return buf;
		if (!len) return 0;
		/* shrink used size to match new total */
		if (len < used) buf->used = len;
		buf->size = len;
		
		return buf;
	}
	else {
		MPT_STRUCT(buffer) *nbuf;
		
		if (!(nbuf = _mpt_memmap(len, (!buf || buf->shared) ? 0 : buf)))
			return 0;
		
		if (buf && buf != nbuf) {
			if (used) memcpy(nbuf+1, buf+1, used);
			if (!(buf->shared--)) munmap(buf, size);
		}
		nbuf->resize = _mpt_buffer_remap;
		nbuf->shared = 0;
		nbuf->size   = len+sizeof(*nbuf);
		nbuf->used   = used;
		
		return nbuf;
	}
}

