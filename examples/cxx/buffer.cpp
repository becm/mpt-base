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
	
	d.insert(3, 4);
	d.set(2, 1);
	std::cout << type(d) << '>' << type(d.elements());
	std::cout << ": " << d.elements() << std::endl;
	
	mpt::Pipe<mpt::array> aq;
	mpt::array tst;
	
	tst.append(std::strlen(txt), txt);
	
	aq.push(tst);
	
	for (int i = 0; i < argc; ++i) {
		buf.write(1, argv[i], strlen(argv[i]) + 1);
	}
	char *v;
	if ((v = std::strrchr(*argv, '/'))) {
		buf.shift(v + 1 - *argv);
	}
	while (buf.get(type(v), &v) > 0) {
		std::cout << v << std::endl;
		buf.advance();
	}
	
	return 0;
}

