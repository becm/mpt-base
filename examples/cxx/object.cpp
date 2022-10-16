/*!
 * test MPT object interface
 */

#include <typeinfo>
#include <iostream>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(node.h)
#include MPT_INCLUDE(config.h)
#include MPT_INCLUDE(message.h)
#include MPT_INCLUDE(convert.h)

#include MPT_INCLUDE(layout.h)

#include MPT_INCLUDE(object.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

#define toString(x) #x

class Double {
	public:
	Double(double d) { _d = d; }
	virtual ~Double() { }
	virtual int f0() { return _d; }
	virtual int f1(int) { return 1; }
	virtual int f2(int) { return 2; }
	virtual int f3(int) { return 3; }
	virtual int f4(int) { return 4; }
	virtual int f5(int) { return 5; }
	virtual int f6(int) { return 6; }
	virtual int f7(int) { return 7; }
	private:
	double _d;
};


class Int {
	public:
	Int(int d) { _d = d; }
	virtual ~Int() { }
	static int (*get)();
	virtual int f0() { return _d; }
	private:
	int _d;
};

// SFINAE test
template <typename T>
class has_get
{
public:
	operator bool () const
	{ return test<T>(0); }
private:
	template <typename C> static bool test(__decltype(&C::get))
	{ return true; }
	template <typename C> static bool test(...)
	{ return false; }
};

extern int main(int , char * const [])
{
	mtrace();
	
	std::cout << "int: " << has_get<Int>() << std::endl;
	std::cout << "dbl: " << has_get<Double>() << std::endl;
	std::cout << toString(hallo) << std::endl;
	
	Double d(46);
	std::cout << d.f0() << std::endl;
	
	mpt::config::root config;
	mpt::convertable *val;
	if ((val = config.get(0))) {
		std::cout << "base type: " << typeid(*val).name() << std::endl;
	}
	config.set("next", "val");
	if ((val = config.get("next"))) {
		std::cout << "next type: " << typeid(*val).name() << std::endl;
	}
	mpt::layout::line *li = new mpt::reference<mpt::layout::line>::type;
	
	
	mpt::layout::graph::axis *ax = new mpt::reference<mpt::layout::graph::axis>::type;
	
	std::cout << "ao: " << ax->type() << std::endl;
	std::cout << "lo: " << li->type() << std::endl;
	
	for (const auto &i : *li) {
		const mpt::property &p = i;
		mpt::color col;
		if (p.val.get(col)) {
			std::cout << "  " << p.name << " = " << col << std::endl;
		} else {
			std::cout << "  " << p.name << " = " << p.val << std::endl;
		}
	}
	
	mpt::object *obj;
	obj = ax;
	std::cout << "type(ao): " << typeid(*obj).name() << std::endl;
	obj = li;
	std::cout << "type(lo): " << typeid(*obj).name() << std::endl;
	
	mpt::object &lr = *li;
	mpt::object::attribute a = lr["x1"];
	mpt::line *l = li;
	
	int old = l->from.x;
	a = "10";
	std::cout << old << " -> " << l->from.x << std::endl;
	
	lr["x2"] = 4.6;
	std::cout << lr["x2"] << std::endl;
	lr["color"] = "#6666";
	
	li->unref();
	ax->unref();
	
	return 0;
}

