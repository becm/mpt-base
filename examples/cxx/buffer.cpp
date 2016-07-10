/*!
 * test buffer/queue object
 */

#include <iostream>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(meta.h)
#include MPT_INCLUDE(queue.h)
#include MPT_INCLUDE(array.h)

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

const char txt[] = "fdsgfdgm dfkhndn djgkh d hdfhsjdfgh df gh dir";

extern int main(int , char * const [])
{
	mtrace();
	
	mpt::Buffer buf;
	mpt::Array<double> d;
	
	MyQueue qu;
	mpt::Pipe<char> cq('a', 4);
	mpt::Pipe<mpt::Buffer> cqp;
	mpt::Queue *raw = &qu;
	
	buf.write(1, txt, strlen(txt));
	
	d.insert(3, 4);
	d.set(2, 1);
	std::cout << mpt::typeIdentifier(d) << '>';
	std::cout << mpt::typeIdentifier(d.slice()) << ": ";
	std::cout << d.slice() << std::endl;
	
	cq.push('b');
	cq.push(0);
	std::cout << cq.data() << std::endl;
	puts(cq.data().base());
	
	cqp.push(buf);
	cqp.shift();
	
	
	buf.peek(2);
	mpt::Slice<uint8_t> data = buf.data();
	
	fwrite(data.base(), data.length(), 1, stdout);
	fputc('\n', stdout);
	
	qu.prepare(456);
	/*
	mpt::array tst;
	
	tst += buf;
	*/
	raw->push(txt, sizeof(txt)-1);
	
	//puts(buf.string());
	
	qu.pushString("hallo");
	qu.append('t');
	qu.shift(0, 1);
	qu.shift(0, 1);
	
	puts(qu.string());
	
	return 0;
}

