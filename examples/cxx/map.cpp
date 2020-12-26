/*!
 * test config/path object
 */

#include <iostream>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(values.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

using namespace mpt;

extern int main(int , char * const [])
{
	mtrace();
	
	map<laydest, reference<cycle> > p;
	reference<cycle> c(new reference<cycle>::type);
	
	p.set(laydest(1,2,3), c);
	p.set(laydest(1,4,3), c);
	
	reference<cycle> *cp = p.get(laydest(1,2,3));
	if (!cp) {
		std::cerr << "missing destination" << std::endl;
		return 1;
	}
	
	double data[] = { 1, 2, 3 };
	c.instance()->modify(1, 'd', data, sizeof(data));
	c.instance()->advance();
	modify(*c.instance(), 0, span<const double>(data + 1, 2));
	modify(*c.instance(), 1, span<const double>(data + 2, 1));
	
	typed_array<reference<cycle>> r = p.values();
	
	for (auto &x : r) {
		cycle *m = x.instance();
		for (auto &s : m->stages()) {
			std::cout << '[';
			for (auto &v : s.rawdata_stage::values()) {
				std::cout << v.element_count() << ' ';
			}
			std::cout << ']' << ' ';
		}
		std::cout << std::endl;
	}
	
	r = p.values(laydest(1,4,3));
	
	return 0;
}
