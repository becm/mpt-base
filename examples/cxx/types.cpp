/*!
 * test config/path object
 */

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(meta.h)
#include MPT_INCLUDE(array.h)

#include <iostream>

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

template <typename T>
void print()
{
	int id = mpt::type_properties<T>::id(true);
	int sid = mpt::type_properties<mpt::span<const T> >::id(true);
	int mid = mpt::type_properties<mpt::span<T> >::id(true);
	int bid = mpt::basetype(id);
	int bsid = mpt::basetype(sid);
	std::cout << id  << ' ' << '<' << bid  << '>' << ' ';
	std::cout << sid << ' ' << '<' << bsid << '>' << ' ';
	std::cout << mid << std::endl;
}
template <typename T>
int type_id(const T &)
{
	return mpt::type_properties<T>::id(true);
}

extern int main(int, char *[])
{
	static const uint8_t fmt[] = {
		mpt::basetype(mpt::type_properties<long>::id(true)),
		0
	};
	mtrace();
	mpt::value v;
	
	int type = mpt::type_traits::add(mpt::type_traits(8));
	
	std::cout << "long: " << fmt << std::endl;
	std::cout << std::hex;
	
	print<int>();
	print<float>();
	print<mpt::array>();
	print<mpt::unique_array<double>>();
	print<mpt::typed_array<double>>();
	print<mpt::metatype::value<double> *>();
	print<mpt::metatype *>();
	print<int *>();
	print<mpt::reference<mpt::metatype> >();
	print<const mpt::reference<mpt::metatype> >();
	
	std::cout << "generic: " << type << " … ";
	mpt::float80 r;
	type_id(r); // register type before slot depletion by loop
	while (true) {
		int curr = mpt::type_traits::add(mpt::type_traits(1));
		if (curr < 0) {
			std::cout << type << std::endl;
			break;
		}
		type = curr;
	}
	type = mpt::type_traits::get("meta")->type;
	std::cout << "meta: " << type << " … ";
	while (true) {
		const mpt::named_traits *curr = mpt::type_traits::add_metatype(0);
		if (!curr) {
			std::cout << type << std::endl;
			break;
		}
		type = curr->type;
	}
	type = mpt::type_traits::get("convertable")->type;
	std::cout << "interface: " << type << " … ";
	while (true) {
		const mpt::named_traits *curr = mpt::type_traits::add_interface(0);
		if (!curr) {
			std::cout << type << std::endl;
			break;
		}
		type = curr->type;
	}
	type = mpt::type_traits::add_basic(24);
	std::cout << "dyn_basic: " << type << " … ";
	while (true) {
		int curr = mpt::type_traits::add_basic(8);
		if (curr < 0) {
			std::cout << type << std::endl;
			break;
		}
		type = curr;
	}
	
	long l = -1;
	v = l;
	std::cout << "long(" << v.type_id() << ") = " << v << std::endl;
	
	unsigned long u = 1;
	v = u;
	std::cout << "ulong(" << v.type_id() << ") = " << v << std::endl;
	
	float f(1.0f / 3);
	std::cout << "float(" << type_id(f) << ") = " << f << std::endl;
	double d = 1.0/3;
	std::cout << "double(" << type_id(d) << ") = " << d << std::endl;
	r = -1.0L / 3;
	std::cout << "float80(" << type_id(r) << ") = " << r << std::endl;
	long double e = 1.0L / 3;
	std::cout << "long double(" << type_id(e) << ") = " << e << std::endl;
	
	mpt::span<const double> t(&d, 1);
	std::cout << "span<" << type_id(*t.begin()) <<">(" << type_id(t) << ") = " << t << std::endl;
	v = t;
	std::cout << "value(<" << v.type_id() << ">) = " << v << std::endl;
	v = static_cast<long double>(r); // convert to native format
	std::cout << "value(<" << v.type_id() << ">) = " << v << std::endl;
	
	return !(r == -e);
}
