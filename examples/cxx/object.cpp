/*!
 * test MPT object interface
 */

#include <typeinfo>
#include <iostream>

#include <mpt/node.h>
#include <mpt/config.h>
#include <mpt/message.h>
#include <mpt/convert.h>

#include <mpt/plot.h>

#include <mpt/object.h>

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

#define toString(x) #x

class Double {
    public:
    Double(double d) { _d = d; }
    virtual int f0(void) { return _d; }
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
    static int (*get)();
    virtual int f0(void) { return _d; }
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

extern int main(int , char * const argv[])
{
	mtrace();
	
	mpt::Config config(argv[1]);
	mpt::metatype *mt;
	Double d(46);
	
	mpt::Output(mpt::LogDebug) << "text" << 56;
	
	mpt::Object ao(new mpt::Axis);
	
	std::cout << "ao: " << ao.type() << std::endl;
	
	std::cout << "int: " << has_get<Int>() << std::endl;
	std::cout << "dbl: " << has_get<Double>() << std::endl;
	std::cout << toString(hallo) << std::endl;
	
	std::cout << d.f0() << std::endl;
	
	mt = config.get(0);
// 	std::cout << "top identifier: " << typeid(*mt).name() << std::endl;
	
	config.set("next", "val");
	mt = config.get("next");
	std::cout << "next identifier: " << typeid(*mt).name() << std::endl;
	
	mpt::Line *li = new mpt::Line;
	mpt::Object lo(li);
	mpt::Object op, *opt;
	
	std::cout << "lo: " << lo.type() << std::endl;
	
	mt = ao;
	std::cout << "type(ao): " << typeid(*mt).name() << std::endl;
	mt = lo;
	std::cout << "type(lo): " << typeid(*mt).name() << std::endl;
	
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
	op.setMeta(0);
	op.setMeta(lo);
	opt = &lo;
	opt->setMeta(lo);
	opt = &op;
	opt->setMeta(lo);
	
	double t = -.3e-5;
	mpt::float80 v = t;
	
	std::cout << t << " " << v.value() << std::endl;
	
	return 0;
}

