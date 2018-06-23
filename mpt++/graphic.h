/*!
 * MPT C++ library
 *  graphic output interface
 */

#ifndef _MPT_GRAPHIC_H
#define _MPT_GRAPHIC_H  @INTERFACE_VERSION@

#include "array.h"

#include "values.h"

__MPT_NAMESPACE_BEGIN

class layout;

struct message;


class graphic
{
public:
	graphic();
	virtual ~graphic();
	
	class hint
	{
	public:
		hint(int = -1, int = -1, int = -1);
		
		bool merge(const hint &, int = 0);
		bool destination(laydest *);
		
		uint8_t match;
		uint8_t lay, grf, wld;
	};
	class mapping;
	
	// layout (de)registration
	virtual int add_layout(layout *, bool = true);
	virtual int remove_layout(const layout *);
	long layout_count() const;
	
	// create new layout
	virtual layout *create_layout();
	
	// mapping helpers
	int target(laydest &, message &, size_t = 0) const;
	metatype *item(message &, size_t = 0) const;
	
	// untracked reference to shedule update
	virtual bool register_update(const reference *, hint = hint());
protected:
	virtual void dispatch_updates();
	reference_array<layout> _layouts;
};

class graphic::mapping : map<laydest, reference_wrapper<cycle> >
{
public:
	int add(valsrc , laydest , int = 0);
	int del(const valsrc *, const laydest * = 0, int = 0) const;
	typed_array<laydest> destinations(valsrc , int = 0) const;
	
	inline const map<laydest, reference_wrapper<class cycle> > &targets() const
	{
		return *this;
	}
	// direct cycle access
	const reference_wrapper<class cycle> *cycle(laydest) const;
	bool set_cycle(laydest, class cycle *);
	
	// modify target elements
	int set_cycles(const span<const reference_wrapper<layout> > &, hint = hint());
	int get_cycles(const span<const reference_wrapper<layout> > &, hint = hint());
	int clear_cycles(hint = hint()) const;
	
	void clear();
protected:
	typed_array<::mpt::mapping> _bind;
};

template<typename T, typename H>
class updates : array
{
public:
	class element
	{
	public:
		inline element(const T &m = T(), const H &h = H()) : data(m), hint(h), used(1)
		{ }
		void invalidate()
		{
			this->~element();
			this->used = 0;
		}
		T data;
		H hint;
		uint32_t used;
	};
	inline span<element> elements() const
	{
		return span<element>(static_cast<element *>(base()), length());
	}
	inline long length() const
	{
		return array::length() / sizeof(element);
	}
	int add(const T &d, const H &h = H())
	{
		element *cmp = static_cast<element *>(base());
		long max = length();
		for (long i = 0; i < max; ++i) {
			if (!cmp[i].used) {
				continue;
			}
			// different data elements
			if (d != cmp[i].data) {
				continue;
			}
			if (cmp[i].hint.merge(h)) {
				++cmp[i].used;
				return 1;
			}
		}
		for (long i = 0; i < max; ++i) {
			if (!cmp[i].used) {
				new (cmp + i) element(d, h);
				return 0;
			}
		}
		if (!(cmp = static_cast<element *>(array::append(sizeof(*cmp))))) {
			return BadOperation;
		}
		new (cmp) element(d, h);
		return 2;
	}
	
	void compress(int mask = 0) {
		element *u = 0, *c = static_cast<element *>(base());
		long len = 0;
		
		for (long i = 0, max = length(); i < max; ++i) {
			if (!c[i].used) {
				if (!u) u = c + i;
				continue;
			}
			for (long j = i + 1; j < max; ++j) {
				if (!c[j].used || c[i].data != c[j].data) {
					continue;
				}
				if (!c[i].hint.merge(c[j].hint, mask)) {
					continue;
				}
				c[i].used += c[j].used;
				c[j].invalidate();
			}
			++len;
			
			if (!u) {
				continue;
			}
			*u = c[i];
			c[i].invalidate();
			
			while (++u < c + i) {
				if (!u->used) {
					break;
				}
			}
		}
		content *d;
		if ((d = _buf.reference())) {
			d->set_length(len * sizeof(*c));
		}
	}
};

__MPT_NAMESPACE_END

std::ostream &operator<<(std::ostream &, const mpt::graphic::hint &);

#endif // _MPT_GRAPHIC_H
