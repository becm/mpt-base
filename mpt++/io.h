/*!
 * MPT I/O instances
 *  queue operations on managed data
 */

#ifndef _MPT_IO_H
#define _MPT_IO_H  @INTERFACE_VERSION@


#include "meta.h"
#include "array.h"

#ifdef _MPT_STREAM_H
# include "object.h"
# include "notify.h"
# include "output.h"
#endif

__MPT_NAMESPACE_BEGIN

struct message;

namespace io {

/*! interface to raw device */
class interface
{
protected:
	inline ~interface() { }
public:
	virtual ssize_t write(size_t , const void *, size_t = 1) = 0;
	virtual ssize_t read(size_t , void *, size_t = 1) = 0;
	
	virtual int64_t pos();
	virtual bool seek(int64_t);
	virtual span<uint8_t> peek(size_t);
	virtual int getchar();
};


/* metatype extension to encode array */
class buffer : public metatype, public iterator, public interface, public encode_array
{
public:
	buffer(array const& = array(0));
	virtual ~buffer();
	
	int convert(int , void *) __MPT_OVERRIDE;
	
	void unref() __MPT_OVERRIDE;
	buffer *clone() const __MPT_OVERRIDE;
	
	int get(int , void *) __MPT_OVERRIDE;
	int advance() __MPT_OVERRIDE;
	int reset() __MPT_OVERRIDE;
	
	ssize_t write(size_t , const void *, size_t = 1) __MPT_OVERRIDE;
	ssize_t read(size_t , void *, size_t = 1) __MPT_OVERRIDE;
	
	int64_t pos() __MPT_OVERRIDE;
	bool seek(int64_t) __MPT_OVERRIDE;
	span<uint8_t> peek(size_t) __MPT_OVERRIDE;
};

#ifdef _MPT_STREAM_H
/* message extension for stream data */
class stream : public input, public object, public output, public interface
{
public:
	stream(const streaminfo * = 0);
	virtual ~stream();
	
	int convert(int , void *) __MPT_OVERRIDE;
	
	void unref() __MPT_OVERRIDE;
	stream *clone() const __MPT_OVERRIDE;
	
	int property(struct property *) const __MPT_OVERRIDE;
	int set_property(const char *, convertable *) __MPT_OVERRIDE;
	
	ssize_t push(size_t , const void *) __MPT_OVERRIDE;
	int sync(int = -1) __MPT_OVERRIDE;
	int await(int (*)(void *, const struct message *) = 0, void * = 0) __MPT_OVERRIDE;
	
	int next(int) __MPT_OVERRIDE;
	int dispatch(event_handler_t , void *) __MPT_OVERRIDE;
	
	ssize_t write(size_t , const void *, size_t part = 1) __MPT_OVERRIDE;
	ssize_t read(size_t , void *, size_t part = 1) __MPT_OVERRIDE;
	int64_t pos() __MPT_OVERRIDE;
	bool seek(int64_t) __MPT_OVERRIDE;
	int getchar() __MPT_OVERRIDE;
	
	bool open(const char *, const char * = "r");
	bool open(void *, size_t , int = ::mpt::stream::Read);
	
	virtual void close();
	
	class dispatch;
protected:
	::mpt::stream *_srm;
	command::array _wait;
	reference<metatype> _ctx;
	uintptr_t _cid;
	int _inputFile;
	uint8_t _idlen;
};
#else
class stream;
#endif

#ifdef _MPT_CONNECTION_H
class socket : public ::mpt::socket
{
public:
	socket(::mpt::socket * = 0);
	virtual ~socket();
	
	int assign(const value *);
	
	virtual reference<class stream> accept();
};
#else
class socket;
#endif


#ifdef _MPT_QUEUE_H
class queue : public interface
{
public:
	queue(size_t = 0);
	virtual ~queue();
	
	/* IODevice interface */
	ssize_t write(size_t , const void *, size_t) __MPT_OVERRIDE;
	ssize_t read(size_t , void *, size_t) __MPT_OVERRIDE;
	
	span<uint8_t> peek(size_t = 0) __MPT_OVERRIDE;
	
	/* queue access */
	virtual bool prepare(size_t);
	
	virtual bool push(const void *, size_t);
	virtual bool pop(void *, size_t);
	
	virtual bool unshift(const void *, size_t);
	virtual bool shift(void *, size_t);
protected:
	::mpt::queue _d;
};
#else
class queue;
#endif

} /* end I/O namespace */

template<> int type_properties<io::interface *>::id(bool);
template<> const struct type_traits *type_properties<io::interface *>::traits();

#ifdef _MPT_QUEUE_H
template <typename T>
class pipe
{
public:
	class instance : public reference<io::queue>::type
	{
	public:
		void unref()
		{
			if (_ref.lower()) {
				return;
			}
			delete this;
		}
	};
	pipe(const T &v, long len)
	{
		if (len <= 0) {
			return;
		}
		ref().instance()->prepare(len * sizeof(T));
		while (len--) {
			push(v);
		}
	}
	inline pipe(const pipe &from)
	{
		*this = from.ref();
	}
	inline pipe & operator=(const pipe &from)
	{
		*this = from.ref();
	}
	inline pipe(instance *q = 0) : _d(q)
	{
		ref();
	}
	~pipe()
	{
		while (pop());
	}
	const reference<instance> &ref()
	{
		if (!_d.instance()) {
			_d.set_instance(new instance);
		}
		return _d;
	}
	bool push(const T & elem)
	{
		io::queue *d;
		if (!(d = _d.instance())) {
			return false;
		}
		uint8_t buf[sizeof(T)];
		T *t = static_cast<T *>(static_cast<void *>(buf));
		new (t) T(elem);
		if (d->push(t, sizeof(T))) {
			return true;
		}
		t->~T();
		return false;
	}
	bool pop(T *data = 0) const
	{
		io::queue *d;
		if (!(d = _d.instance())) {
			return false;
		}
		uint8_t buf[sizeof(T)];
		T *t = static_cast<T *>(static_cast<void *>(buf));
		if (!d->pop(t, sizeof(T))) {
			return false;
		}
		if (data) {
			*data = *t;
		} else {
			t->~T();
		}
		return true;
	}
	bool unshift(const T & elem)
	{
		io::queue *d;
		if (!(d = _d.instance())) {
			return false;
		}
		uint8_t buf[sizeof(T)];
		T *t = static_cast<T *>(static_cast<void *>(buf));
		new (t) T(elem);
		if (d->unshift(t, sizeof(T))) {
			return true;
		}
		t->~T();
		return false;
	}
	bool shift(T *data = 0) const
	{
		io::queue *d;
		if (!(d = _d.instance())) {
			return false;
		}
		uint8_t buf[sizeof(T)];
		T *t = static_cast<T *>(static_cast<void *>(buf));
		if (!d->shift(t, sizeof(T))) {
			return false;
		}
		if (data) {
			*data = *t;
		} else {
			t->~T();
		}
		return true;
	}
	span<T> elements()
	{
		io::queue *d;
		if (!(d = _d.instance())) {
			return span<T>(0, 0);
		}
		span<uint8_t> r = d->peek();
		return span<T>((T *) r.begin(), r.size() / sizeof(T));
	}
protected:
	reference<instance> _d;
};
#endif

__MPT_NAMESPACE_END

#endif /* _MPT_QUEUE_H */
