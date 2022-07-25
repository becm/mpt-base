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
	virtual span<const uint8_t> peek(size_t);
	virtual int getchar();
	
	static const named_traits *get_traits();
};

/* metatype extension to encode array */
class buffer : public iterator, public interface, public encode_array
{
public:
	buffer(array const& = array(0));
	virtual ~buffer();
	
	class metatype;
	
	const struct value *value() __MPT_OVERRIDE;
	int advance() __MPT_OVERRIDE;
	int reset() __MPT_OVERRIDE;
	
	ssize_t write(size_t , const void *, size_t = 1) __MPT_OVERRIDE;
	ssize_t read(size_t , void *, size_t = 1) __MPT_OVERRIDE;
	
	int64_t pos() __MPT_OVERRIDE;
	bool seek(int64_t) __MPT_OVERRIDE;
	span<const uint8_t> peek(size_t) __MPT_OVERRIDE;
protected:
	struct value _value;
	span<const char> _record;
};

class buffer::metatype : public ::mpt::metatype, public buffer
{
public:
	static metatype *create(const array *);
	
	int convert(int , void *) __MPT_OVERRIDE;
	metatype *clone() const __MPT_OVERRIDE;
protected:
	metatype(const array &);
};
#ifdef _MPT_STREAM_H
/* message extension for stream data */
class stream : public object, public output, public interface
{
public:
	stream(const streaminfo * = 0);
	virtual ~stream();
	
	class input;
	
	int property(struct property *) const __MPT_OVERRIDE;
	int set_property(const char *, convertable *) __MPT_OVERRIDE;
	
	ssize_t push(size_t , const void *) __MPT_OVERRIDE;
	int sync(int = -1) __MPT_OVERRIDE;
	int await(int (*)(void *, const struct message *) = 0, void * = 0) __MPT_OVERRIDE;
	
	ssize_t write(size_t , const void *, size_t part = 1) __MPT_OVERRIDE;
	ssize_t read(size_t , void *, size_t part = 1) __MPT_OVERRIDE;
	int64_t pos() __MPT_OVERRIDE;
	bool seek(int64_t) __MPT_OVERRIDE;
	int getchar() __MPT_OVERRIDE;
	
	bool open(const char *, const char * = "r");
	bool open(void *, size_t , int = ::mpt::stream::Read);
	
	virtual void close();
protected:
	::mpt::stream *_srm;
	command::array _wait;
	reference<metatype> _ctx;
	uintptr_t _cid;
	int _inputFile;
	uint8_t _idlen;
	
	class dispatch
	{
	public:
		dispatch(io::stream &, event_handler_t c, void *);
		int process(const struct message *msg) const;
		static int stream_dispatch(void *, const struct message *);
	protected:
		io::stream &srm;
		event_handler_t cmd;
		void *arg;
	};
	int dispatch(const class dispatch &);
};
class stream::input : public ::mpt::input, public stream
{
public:
	static input *create(const streaminfo * = 0);
	
	int convert(int , void *) __MPT_OVERRIDE;
	::mpt::input *clone() const __MPT_OVERRIDE;
	
	int next(int) __MPT_OVERRIDE;
	int dispatch(event_handler_t , void *) __MPT_OVERRIDE;
protected:
	input(const streaminfo *);
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
	
	virtual reference<class stream::input> accept();
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
	
	span<const uint8_t> peek(size_t = 0) __MPT_OVERRIDE;
	
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
		}
		t->~T();
		return true;
	}
	span<T> elements()
	{
		io::queue *d;
		if (!(d = _d.instance())) {
			return span<T>(0, 0);
		}
		span<const uint8_t> r = d->peek();
		return span<T>((T *) r.begin(), r.size() / sizeof(T));
	}
protected:
	reference<instance> _d;
};
#endif

__MPT_NAMESPACE_END

#endif /* _MPT_QUEUE_H */
