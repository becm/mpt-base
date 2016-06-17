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
	
	mpt::Object ao(new mpt::Axis);
	
	std::cout << "ao: " << ao.type() << std::endl;
	std::cout << "axis type: " << typeid(*ao.pointer()).name() << std::endl;
	
	std::cout << "int: " << has_get<Int>() << std::endl;
	std::cout << "dbl: " << has_get<Double>() << std::endl;
	std::cout << toString(hallo) << std::endl;
	
	Double d(46);
	std::cout << d.f0() << std::endl;
	
	mpt::Config config;
	mpt::metatype *mt;
	if ((mt = config.get(0))) {
		std::cout << "base type: " << typeid(*mt).name() << std::endl;
	}
	config.set("next", "val");
	mt = config.get("next");
	std::cout << "next type: " << typeid(*mt).name() << std::endl;
	
	mpt::Line *li = new mpt::Reference<mpt::Line>::instance;
	mpt::Object lo(li);
	mpt::Object op, *opt;
	
	std::cout << "lo: " << lo.type() << std::endl;
	
	for (auto &i : *static_cast<const mpt::object *>(li)) {
		std::cout << "  " << i.name << " = " << i.val << std::endl;
	}
	
	mpt::object *obj;
	obj = ao.pointer();
	std::cout << "type(ao): " << typeid(*obj).name() << std::endl;
	obj = lo.pointer();
	std::cout << "type(lo): " << typeid(*obj).name() << std::endl;
	
	mpt::Axis *na = new mpt::Axis;
	
	mpt::Object nao(na);
	
	std::cout << "type(na): " << typeid(*na).name() << std::endl;;
	
	nao.setName("axis");
	
	std::cout << "nao.name() = " << nao.name() << std::endl;
	
	mpt::Property prop = lo["x1"];
	mpt::line *l = li;
	
	std::cout << l->from.x << std::endl;
	prop = "10";
	std::cout << l->from.x << std::endl;
	
	lo["x2"] = 4;
	lo["color"] = "#6666";
	
	op = lo;
	
	// failing assignments for empty/same metatype
	op.setPointer(0);
	op.setPointer(lo.pointer());
	opt = &lo;
	opt->setPointer(lo.pointer());
	opt = &op;
	opt->setPointer(lo.pointer());
	
	double t = -.3e-5;
	mpt::float80 v = t;
	
	std::cout << t << " " << v.value() << std::endl;
	
	return 0;
}

