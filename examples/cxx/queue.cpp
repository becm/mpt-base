/*!
 * test config/path object
 */

#include <cstring>
#include <iostream>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(queue.h)
#include MPT_INCLUDE(io.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

class MyQueue : public mpt::io::queue
{
public:
	void add(const char *, int = -1);
	void add(char);
	mpt::span<const char> string();
};

void MyQueue::add(const char *txt, int len)
{
	mpt::io::queue::push((const void *) txt, (len < 0) ? std::strlen(txt) : len);
}
void MyQueue::add(char c)
{
	mpt::io::queue::push(&c, 1);
}

mpt::span<const char> MyQueue::string()
{
	mpt::span<const uint8_t> d = peek();
	const char *base = reinterpret_cast<const char *>(d.begin());
	return mpt::span<const char>(base, d.size());
}

extern int main(int argc, char * const argv[])
{
	mtrace();
	
	mpt::pipe<char> p;
	p.push('a');
	p.push('l');
	p.push('l');
	p.push('o');
	p.unshift('h');
	std::cout << p.elements() << std::endl;
	
	
	mpt::pipe<int> i(5, 8);
	std::cout << i.elements() << std::endl;
	
	mpt::pipe<char> cq('a', 4);
	cq.push('b');
	std::cout << cq.elements() << std::endl;
	
	
	MyQueue qu;
	qu.prepare(456);
	mpt::io::queue *raw = &qu;
	
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
