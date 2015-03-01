/*!
 * test buffer/queue object
 */


#include <stdio.h>
#include <string.h>

#include <sys/uio.h>

#include <mpt/queue.h>
#include <mpt/array.h>
#include <mpt/layout.h>

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
	char *base = (char *) data().base();
	base[data().len()] = 0;
	return base;
}

const char txt[] = "fdsgfdgm dfkhndn djgkh d hdfhsjdfgh df gh dir";

extern int main(int argc, char *argv[])
{
	mtrace();
	
	mpt::Buffer buf;
	mpt::Array<double> d;
	
	MyQueue  qu;
	mpt::Pipe<char> cq('a', 4);
	mpt::Pipe<mpt::Buffer> cqp;
	mpt::Queue *raw = &qu;
	
	mpt::Reference<mpt::Queue> rq = cq.ref();
	
	buf.write(1, txt, strlen(txt));
	
	d.insert(3, 4);
	for (mpt::Array<double>::iterator it = d.begin(), end = d.end(); it != end; ++it) {
		printf("%lf\n", *it);
	}
	
	cq.push('b');
	cq.push(0);
	puts(cq.data().base());
	
	cqp.push(buf);
	cqp.shift();
	
	
	buf.peek(2);
	mpt::Slice<uint8_t> data = buf.data();
	
	fwrite(data.base(), data.len(), 1, stdout);
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

