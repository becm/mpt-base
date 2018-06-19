/*!
 * test config/path object
 */

#include <cstring>
#include <iostream>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(queue.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

class MyQueue : public mpt::Queue
{
public:
	void add(const char *, int = -1);
	void add(char);
	const char *string();
};

void MyQueue::add(const char *txt, int len)
{
	Queue::push((const void *) txt, (len < 0) ? std::strlen(txt) : len);
}
void MyQueue::add(char c)
{
	Queue::push(&c, 1);
}

const char *MyQueue::string()
{
	if (!prepare(1)) return 0;
	mpt::span<uint8_t> d = peek();
	char *base = reinterpret_cast<char *>(d.base());
	base[d.length()] = 0;
	return base;
}

extern int main(int argc, char * const argv[])
{
	mtrace();
	
	mpt::Pipe<char> p;
	p.push('a');
	p.push('l');
	p.push('l');
	p.push('o');
	p.unshift('h');
	std::cout << p.elements() << std::endl;
	
	
	mpt::Pipe<int> i(5, 8);
	std::cout << i.elements() << std::endl;
	
	mpt::Pipe<char> cq('a', 4);
	cq.push('b');
	std::cout << cq.elements() << std::endl;
	
	
	MyQueue qu;
	qu.prepare(456);
	mpt::Queue *raw = &qu;
	
	raw->push("hallo", 5);
	
	qu.add('{');
	for (int i = 1; i < argc; ++i) {
		qu.add(argv[i]);
	}
	qu.add('}');
	qu.add("hallo");
	qu.add('t');
	qu.shift(0, 1);
	qu.shift(0, 1);
	
	std::cout << qu.string() << std::endl;
	
	return 0;
}
