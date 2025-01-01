/*!
 * test MPT object operations
 */

#include <typeinfo>
#include <iostream>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(layout.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

extern int main(int , char * const [])
{
	mtrace();
	
	// use graphic items
	mpt::layout::line li;
	mpt::layout::graph::axis ax;
	
	std::cout << "ao: " << ax.type() << std::endl;
	std::cout << "lo: " << li.type() << std::endl;
	
	// display properties of line
	for (const auto &i : li) {
		const mpt::property &p = i;
		mpt::color col;
		if (p.val.get(col)) {
			std::cout << "  " << p.name << " = " << col << std::endl;
		} else {
			std::cout << "  " << p.name << " = " << p.val << std::endl;
		}
	}
	
	// validate object instance dispatch
	mpt::object *obj;
	obj = &ax;
	std::cout << "type(ao): " << typeid(*obj).name() << std::endl;
	obj = &li;
	std::cout << "type(lo): " << typeid(*obj).name() << std::endl;
	
	// change line item attribute
	double x1 = li.from.x;
	mpt::object::attribute a = li["x1"];
	a = "10";
	std::cout << x1 << " -> " << li.from.x << std::endl;
	
	// alternate access types for line item
	mpt::line &l = li;
	double x2 = l.to.x;
	mpt::object &lr = li;
	lr["x2"] = 4.6;
	std::cout << x2 << " -> " << l.to.x << std::endl;
	mpt::color c = l.color;
	lr["color"] = "#6666";
	std::cout << c << " -> " << l.color << std::endl;
	
	return 0;
}

