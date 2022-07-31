/*
 * MPT C++ library
 *   polyline operations
 */

#include <cstdlib>
#include <limits>

#include <sys/uio.h>

#include "types.h"

#include "values.h"

__MPT_NAMESPACE_BEGIN

int apply_data(point<double> *dest, const span<const linepart> &pts, const transform &tr, span<const value_store> st)
{
	int dim, proc = 0;
	
	const linepart *lp = pts.begin();
	int plen = pts.size();
	
	dim = tr.dimensions();
	const struct type_traits *traits = type_properties<double>::traits();
	
	for (int i = 0; i < dim; i++) {
		const value_store *val;
		
		if (!(val = st.nth(i))) {
			continue;
		}
		const array::content *d = val->data();
		if (!d || (traits != d->content_traits())) {
			continue;
		};
		long max;
		if (!(max = d->length() / traits->size)) {
			continue;
		}
		const double *from = static_cast<__decltype(from)>(d->data());
		linepart tmp;
		point<double> *to = dest;
		
		if (lp) {
			for (int j = 0; j < plen; j++) {
				if (max < lp[j].usr) {
					tmp = lp[j];
					tmp.usr = max;
					tr.apply(i, tmp, to, from);
					break;
				}
				tr.apply(i, lp[j], to, from);
				to   += lp[j].usr;
				from += lp[j].raw;
			}
		}
		else {
			if (max < plen) {
				plen = max;
			}
			while (plen > std::numeric_limits<__decltype(tmp.usr)>::max()) {
				tmp.usr = tmp.raw = std::numeric_limits<__decltype(tmp.usr)>::max();
				plen -= tmp.usr;
				tr.apply(i, tmp, to, from);
				to   += tmp.usr;
				from += tmp.raw;
			}
			tmp.usr = tmp.raw = plen;
			tr.apply(i, tmp, to, from);
		}
		++proc;
	}
	return proc;
}

bool polyline::set(const transform &tr, span<const value_store> src)
{
	const struct type_traits *traits = type_properties<double>::traits();
	
	// generate parts data
	long max = maxsize(src, traits);
	if (!max || !_vis.set(max)) {
		return false;
	}
	const value_store *val = src.begin();
	for (long i = 0, max = src.size(); i < max; ++i) {
		const array::content *d = val->data();
		if (!d || d->content_traits() != traits) {
			continue;
		}
		long length = d->length() / traits->size;
		_vis.apply(tr, i, span<const double>(static_cast<const double *>(d->data()), length));
		++val;
	}
	// prepare target data
	if (!(max = _vis.length_user())) {
		_values.resize(0);
		return false;
	}
	if (!_values.resize(max)) {
		_values.resize(0);
		return false;
	}
	// set start values
	point *pts = _values.begin();
	::mpt::point<double> z = tr.zero();
	copy(max, &z, 0, pts, 1);
	
	// modify according to transformation
	apply_data(pts, _vis.elements(), tr, src);
	
	return true;
}

// polyline iterator operations
polyline::iterator polyline::begin() const
{
	return polyline::iterator(_vis.elements(), _values.begin());
}
polyline::iterator polyline::end() const
{
	return polyline::iterator(span<const linepart>(_vis.end(), 0), _values.end());
}
polyline::iterator &polyline::iterator::operator++()
{
	if (!_parts.size()) {
		return *this;
	}
	const linepart *curr = _parts.begin();
	_points += curr->usr;
	_parts.skip(1);
	return *this;
}

// polyline part operations
polyline::part polyline::iterator::operator *() const
{
	if (!_parts.size()) {
		return part(linepart(0), 0);
	}
	const linepart *curr = _parts.begin();
	return part(*curr, _points);
}
span<const polyline::point> polyline::part::points() const
{
	size_t len = _part.usr;
	const point *pts = _pts;
	if (_part._cut) {
		++pts;
		--len;
	}
	if (_part._trim) {
		--len;
	}
	return span<const point>(pts, len);
}
span<const polyline::point> polyline::part::line() const
{
	return span<const point>(_pts, _part.usr);
}

__MPT_NAMESPACE_END

