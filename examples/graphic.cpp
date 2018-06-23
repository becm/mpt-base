/*!
 * instance of MPT graphic
 */

#include <iostream>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(graphic.h)
#include MPT_INCLUDE(layout.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

class graphic : public mpt::graphic, public mpt::updates<const mpt::reference *, mpt::graphic::hint>
{
public:
	bool register_update(const mpt::reference *, hint) __MPT_OVERRIDE;
	void dispatch_updates() __MPT_OVERRIDE;
};
bool graphic::register_update(const mpt::reference *r, hint h)
{
	return add(r, h);
}
void graphic::dispatch_updates()
{
	for (auto &e : elements()) {
		const mpt::layout *l = dynamic_cast<const mpt::layout *>(e.data);
		if (l) {
			std::cout << l->alias() << ": " << e.hint << std::endl;
		} else {
			std::cout << e.data << ": " << e.hint << std::endl;
		}
		e.invalidate();
	}
}

int main()
{
	mtrace();
	
	graphic g;
	
	mpt::layout *l = g.create_layout();
	mpt::meta_value<int> m(0);
	
	l->set("name", "lay");
	
	int pos = g.add_layout(l);
	g.register_update(l,  mpt::graphic::hint(pos, 4, 3));
	g.register_update(l,  mpt::graphic::hint(pos, 6, 8)); // unique hint
	g.register_update(l,  mpt::graphic::hint(pos, 4));    // overrides specific entry
	g.register_update(&m, mpt::graphic::hint(pos, 4));    // different data value
	
	g.dispatch_updates();
	g.compress();
	
	g.remove_layout(l);
}
