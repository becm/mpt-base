/*
 * MPT C++ library
 *   metatype creator override
 */

#include "meta.h"

__MPT_NAMESPACE_BEGIN
class default_metatype : public mpt::metatype
{
public:
	void unref() __MPT_OVERRIDE
	{ };
	uintptr_t addref() __MPT_OVERRIDE
	{
		return 1;
	};
	mpt::metatype *clone () const __MPT_OVERRIDE
	{
		const metatype *mt = this;
		return const_cast<mpt::metatype *>(mt);
	}
	virtual ~default_metatype()
	{ }
};
__MPT_NAMESPACE_END

// metatype creator override
extern "C" mpt::metatype *mpt_meta_new(const mpt::value *val)
{
	if (!val) {
		static mpt::default_metatype empty;
		return &empty;
	}
	return mpt::metatype::create(*val);
}

