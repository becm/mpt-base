/*!
 * test buffer/queue object
 */

#include <iostream>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(stream.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif


class MyQueue : public mpt::Queue
{
    public:
	void pushString(const char *, int = -1);
	const char *string();
	void append(char);
};

void MyQueue::pushString(const char *txt, int len)
{ Queue::push((const void *) txt, (len < 0) ? strlen(txt) : len); }

void MyQueue::append(char c)
{
	Queue::push(&c, 1);
}

const char *MyQueue::string()
{
	if (!prepare(1)) return 0;
	mpt::Slice<uint8_t> d = peek();
	char *base = reinterpret_cast<char *>(d.base());
	base[d.length()] = 0;
	return base;
}

template <typename T>
uint8_t type(const T &)
{
	return mpt::basetype(mpt::typeinfo<T>::id());
}

const char txt[] = "fdsgfdgm dfkhndn djgkh d hdfhsjdfgh df gh dir";

extern int main(int argc, char * const argv[])
{
	mtrace();
	
	mpt::Buffer buf;
	mpt::Array<double> d;
	
	MyQueue qu;
	mpt::Pipe<char> cq('a', 4);
	mpt::Pipe<mpt::Buffer> cqp;
	mpt::Queue *raw = &qu;
	
	d.insert(3, 4);
	d.set(2, 1);
	std::cout << type(d) << '>' << type(d.slice());
	std::cout << ": " << d.slice() << std::endl;
	
	cq.push('b');
	std::cout << cq.data() << std::endl;
	
	cqp.push(buf);
	cqp.shift();
	
	
	qu.prepare(456);
	/*
	mpt::array tst;
	
	tst += buf;
	*/
	raw->push(txt, sizeof(txt) - 1);
	for (int i = 0; i < argc; ++i) {
		buf.write(1, argv[i], strlen(argv[i]) + 1);
	}
	buf.shift(2);
	char *v;
	while (buf.get(type(v), &v) > 0) {
		std::cout << v << std::endl;
		buf.advance();
	}
	
	qu.pushString("hallo");
	qu.append('t');
	qu.shift(0, 1);
	qu.shift(0, 1);
	
	std::cout << qu.string() << std::endl;
	
	return 0;
}

