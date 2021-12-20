/*!
 * MPT core library
 *   control and initializer for mmap-based data.
 */

#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>

#include <sys/mman.h>

#include "types.h"

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

static int _mpt_buffer_map_psize = 0;

MPT_STRUCT(bufferData)
{
	MPT_STRUCT(refcount) _ref;
	size_t _align;
	size_t _total;
	int    _flags;
	
	uint8_t _pad[8 * sizeof(void *)
	           - sizeof(MPT_STRUCT(refcount))
	           - sizeof(size_t)
	           - sizeof(size_t)
	           - sizeof(int)
	           - sizeof(MPT_STRUCT(buffer))];
	
	MPT_STRUCT(buffer) _buf;
};

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
		if (!_mpt_buffer_map_psize
		    || (_mpt_buffer_map_psize = sysconf(_SC_PAGESIZE)) < 1) {
			return 0;
		}
		if (len > (size_t) _mpt_buffer_map_psize) {
			munmap(((uint8_t *) base) + _mpt_buffer_map_psize, len - _mpt_buffer_map_psize);
		}
		(void) mprotect(base, _mpt_buffer_map_psize, PROT_NONE);
		base = mmap(base, len, MPT_MMAP_FLAGS, MPT_MMAP_TYPE, devzero, 0);
	}
#if !defined(MAP_ANONYMOUS)
	(void) close(devzero);
#endif
	/* memory mapping failed */
	return (base == MAP_FAILED) ? 0 : base;
}
/* reference interface */
static void _mpt_buffer_map_unref(MPT_INTERFACE(buffer) *ref)
{
	MPT_STRUCT(bufferData) *buf = MPT_baseaddr(bufferData, ref, _buf);
	const MPT_STRUCT(type_traits) *traits;
	void (*fini)(void *);
	size_t size;
	if (mpt_refcount_lower(&buf->_ref)) {
		return;
	}
	if ((traits = buf->_buf._content_traits)
	    && (fini = traits->fini)
	    && (size = traits->size)) {
		size_t len = buf->_buf._size;
		len -= len % size;
		uint8_t *ptr = (void *) (buf + 1);
		size_t pos;
		for (pos = 0; pos < len; pos += size) {
			fini(ptr + pos);
		}
	}
	munmap(buf, buf->_total);
}
static uintptr_t _mpt_buffer_map_ref(MPT_INTERFACE(buffer) *ref)
{
	MPT_STRUCT(bufferData) *buf = MPT_baseaddr(bufferData, ref, _buf);
	return mpt_refcount_raise(&buf->_ref);
}
static MPT_STRUCT(buffer) *_mpt_buffer_map_detach(MPT_STRUCT(buffer) *ptr, size_t len)
{
	MPT_STRUCT(bufferData) *buf = MPT_baseaddr(bufferData, ptr, _buf);
	MPT_STRUCT(buffer) *next;
	size_t old;
	
	if (buf->_ref._val == 1
	 && !(buf->_flags & MPT_ENUM(BufferImmutable))) {
		return &buf->_buf;
	}
	/* only detach raw buffer */
	if (buf->_buf._content_traits) {
		errno = ENOTSUP;
		return 0;
	}
	/* remuve immutability flag on clone */
	if (!(next = _mpt_buffer_map(len, buf->_flags & ~MPT_ENUM(BufferImmutable)))) {
		return 0;
	}
	
	old = buf->_buf._used;
	if (old > len) {
		old = len;
	}
	if (old) {
		memcpy(next + 1, buf + 1, old);
		next->_used = old;
	}
	
	_mpt_buffer_map_unref((void *) &buf->_buf);
	
	return next;
}
static uint32_t _mpt_buffer_map_flags(const MPT_STRUCT(buffer) *ptr)
{
	MPT_STRUCT(bufferData) *buf = MPT_baseaddr(bufferData, ptr, _buf);
	int flags = buf->_ref._val > 1 ? MPT_ENUM(BufferShared) : 0;
	return flags | buf->_flags | MPT_ENUM(BufferMapped);
}
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
extern MPT_STRUCT(buffer) *_mpt_buffer_map(size_t len, int flags)
{
	static const MPT_INTERFACE_VPTR(buffer) _mpt_buffer_map_vptr = {
		_mpt_buffer_map_flags,
		_mpt_buffer_map_unref,
		_mpt_buffer_map_ref,
		_mpt_buffer_map_detach
	};
	MPT_STRUCT(bufferData) *buf;
	size_t align;
	
	/* get page table size & check range */
	if (!_mpt_buffer_map_psize
	    || (_mpt_buffer_map_psize = sysconf(_SC_PAGESIZE)) < 1) {
		return 0;
	}
	if (SIZE_MAX - sizeof(*buf) - _mpt_buffer_map_psize < len) {
		errno = ERANGE;
		return 0;
	}
	len += sizeof(*buf);
	
	if ((align = len % _mpt_buffer_map_psize)) {
		len  += _mpt_buffer_map_psize - align;
	}
	if (!(buf = _mpt_memmap(len, 0))) {
		return 0;
	}
	buf->_ref._val = 1;
	buf->_align = _mpt_buffer_map_psize;
	buf->_total = len;
	buf->_flags = flags & MPT_ENUM(BufferFlagsUser);
	
	buf->_buf._vptr = &_mpt_buffer_map_vptr;
	buf->_buf._content_traits = 0;
	buf->_buf._size = len - sizeof(*buf);
	buf->_buf._used = 0;
	
	return &buf->_buf;
}
