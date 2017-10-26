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
# define MAP_ANONYMOUS  MAP_ANON
#endif

#if defined(MAP_ANONYMOUS)
# define MPT_MMAP_TYPE  (MAP_PRIVATE | MAP_ANONYMOUS)
#else
# include <fcntl.h>
# include <sys/stat.h>
# define MPT_MMAP_TYPE  MAP_PRIVATE
#endif

#define MPT_MMAP_FLAGS  (PROT_READ | PROT_WRITE)

static int ptsize = 0;

/*!
 * \ingroup mptArray
 * \brief memory mapped data
 * 
 * Wrapper for platform abstraction.
 * 
 * \param len  size of mmap data
 * \param base start address hint
 * 
 * \return mapped data start address
 */
extern void *_mpt_memmap(size_t len, void *base)
{
	int devzero = -1;
#if !defined(MAP_ANONYMOUS)
	if ((devzero = open("/dev/null", O_RDWR)) < 0) {
		return 0;
	}
#endif
	/* NULL page mapping -> memory penalty for shitty system */
	if (!(base = mmap(base, len, MPT_MMAP_FLAGS, MPT_MMAP_TYPE, devzero, 0))) {
		/* get page table size & check range */
		if (!ptsize || (ptsize = sysconf(_SC_PAGESIZE)) < 1) {
			return 0;
		}
		if (len > (size_t) ptsize) munmap(((uint8_t *) base) + ptsize, len - ptsize);
		(void) mprotect(base, ptsize, PROT_NONE);
		base = mmap(base, len, MPT_MMAP_FLAGS, MPT_MMAP_TYPE, devzero, 0);
	}
#if !defined(MAP_ANONYMOUS)
	(void) close(devzero);
#endif
	/* memory mapping failed */
	return (base == MAP_FAILED) ? 0 : base;
}

static void _mpt_buffer_map_unref(MPT_INTERFACE(reference) *ref)
{
	MPT_STRUCT(buffer) *buf = (void *) ref;
	
	if (mpt_refcount_lower(&buf->_ref)) {
		return;
	}
	munmap(buf, sizeof(*buf) + buf->_size);
}
static uintptr_t _mpt_buffer_map_ref(MPT_INTERFACE(reference) *ref)
{
	MPT_STRUCT(buffer) *buf = (void *) ref;
	return mpt_refcount_raise(&buf->_ref);
}
static MPT_STRUCT(buffer) *_mpt_buffer_map_detach(MPT_STRUCT(buffer) *buf, long len)
{
	long align, max;
	
	/* deny non-exclusive modification */
	if (buf->_ref._val > 1) {
		errno = ENOTSUP;
		return 0;
	}
	if (len < 0) {
		return buf;
	}
	/* deny mapping enlarge (MAP_FIXED is ... strange?) */
	if ((size_t) len > buf->_size) {
		errno = EINVAL;
		return 0;
	}
	/* discard tailing data */
	len += sizeof(*buf);
	max = sizeof(*buf) + buf->_size;
	if ((align = len % ptsize)) {
		len += ptsize - align;
	}
	if (len < max) {
		if (munmap(((uint8_t *) buf) + len, max - len) < 0) {
			return 0;
		}
		len -= sizeof(*buf);
		buf->_size = len;
		if ((size_t) len < buf->_used) {
			buf->_used = len;
		}
	}
	return buf;
}
static int _mpt_buffer_map_type(const MPT_STRUCT(buffer) *buf)
{
	(void) buf;
	return 0;
}
static const MPT_INTERFACE_VPTR(buffer) _mpt_buffer_map_vptr = {
	{ _mpt_buffer_map_unref, _mpt_buffer_map_ref },
	_mpt_buffer_map_detach,
	_mpt_buffer_map_type
};
/*!
 * \ingroup mptArray
 * \brief mmap buffer data
 * 
 * Buffer data with memory mapped data backend.
 * 
 * \param size  requested new buffer size
 * 
 * \return new buffer sattisfying size
 */
extern MPT_STRUCT(buffer) *_mpt_buffer_map(size_t len)
{
	MPT_STRUCT(buffer) *buf;
	size_t align;
	
	/* get page table size & check range */
	if (!ptsize || (ptsize = sysconf(_SC_PAGESIZE)) < 1) {
		return 0;
	}
	if (SIZE_MAX - sizeof(*buf) - ptsize < len) {
		errno = ERANGE;
		return 0;
	}
	len += sizeof(*buf);
	
	if ((align = len % ptsize)) {
		len += ptsize - align;
	}
	if (!(buf = _mpt_memmap(len, 0))) {
		return 0;
	}
	buf->_vptr = &_mpt_buffer_map_vptr;
	buf->_ref._val = 1;
	buf->_size = len - sizeof(*buf);
	buf->_used = 0;
	
	return buf;
}
