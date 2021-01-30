/*
 * MPT C++ library
 *   identifier operations
 */

#include "core.h"

__MPT_NAMESPACE_BEGIN

// identifier operations
identifier::identifier(size_t total)
{
	mpt_identifier_init(this, total);
}
identifier::identifier(const identifier &id)
{
	mpt_identifier_init(this, sizeof(*this));
	mpt_identifier_copy(this, &id);
}
bool identifier::equal(const char *name, int nlen) const
{
	return mpt_identifier_compare(this, name, nlen) ? false : true;
}
identifier &identifier::operator=(const identifier &id)
{
	mpt_identifier_copy(this, &id);
	return *this;
}
bool identifier::set_name(const char *name, int nlen)
{
	return (mpt_identifier_set(this, name, nlen)) ? true : false;
}
const char *identifier::name() const
{
	if (_type != 'c') {
		return 0;
	}
	return static_cast<const char *>(mpt_identifier_data(this));
}

__MPT_NAMESPACE_END
