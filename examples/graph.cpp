/*!
 * instance of MPT client
 */

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include <cassert>
#include <iostream>

#include MPT_INCLUDE(layout.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

int main(int argc, char * const argv[])
{
	mtrace();
	
	mpt::item_group other;
	mpt::layout::graph g;
	mpt::identifier id;
	
	id.set_name("ax");
	other.append(&id, new mpt::layout::graph::axis);
	
	// can be used for default element lookup
	id.set_name("w1");
	g.append(&id, new mpt::reference<mpt::layout::graph::world>::type);
	
	// initial values for new world
	mpt::world w;
	mpt::mpt_color_parse(&w.color, "red");
	// no support for refcount, name lookup required
	id.set_name("w2");
	g.append(&id, new mpt::layout::graph::world(&w));
	
	// look for specific elements by name
	if (argc > 1) {
		g["worlds"] = argv[1];
	}
	if (argc > 2) {
		g["axes"] = argv[2];
	}
	// bind graph elements
	mpt::collection_relation top(other);
	mpt::collection_relation rel(g, &top);
	g.bind(&rel);
	
	int fid = mpt::type_properties<mpt::fpoint>::id(true);
	int did = mpt::type_properties<mpt::dpoint>::id(true);
	std::cout << "fpoint type: " << fid << std::endl;
	std::cout << "dpoint type: " << did << std::endl;
	
	assert(fid == mpt::type_properties<mpt::point<float>>::id(true));
	assert(did == mpt::type_properties<mpt::point<double>>::id(true));
	
	// add additional world element
	mpt::item<mpt::layout::graph::data> *d = g.add_world(0, "w3");
	mpt::mpt_color_parse(&d->instance()->world.instance()->color, "cyan");
	
	// print world elements
	for (auto &w : g.worlds()) {
		mpt::object &o = *w.instance()->world.instance();
		mpt::color col;
		const mpt::value &val = o["color"];
		val.get(col);
		std::cout << w.name() << ": " << col << std::endl;
	}
	for (auto &a : g.axes()) {
		mpt::object &o = *a.instance();
		std::cout << a.name() << ": " << o["tlen"] << std::endl;
	}
}
