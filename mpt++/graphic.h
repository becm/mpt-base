/*!
 * MPT C++ library
 *  graphic output interface
 */

#ifndef _MPT_GRAPHIC_H
#define _MPT_GRAPHIC_H  @INTERFACE_VERSION@

#include "array.h"

#include "values.h"

__MPT_NAMESPACE_BEGIN

class Cycle;
class Graph;
class Layout;

struct message;

class UpdateHint
{
public:
    UpdateHint(int = -1, int = -1, int = -1);

    bool merge(const UpdateHint &, int = 0);
    bool destination(laydest *);

    uint8_t match;
    uint8_t lay, grf, wld;
};

class Mapping : Map<laydest, Reference<Cycle> >
{
public:
    int add(valsrc , laydest , int = 0);
    int del(const valsrc *, const laydest * = 0, int = 0) const;
    Array<laydest> destinations(valsrc , int = 0) const;

    inline const Map<laydest, Reference<Cycle> > &targets() const
    { return *this; }

    // direct cycle access
    const Reference<Cycle> *cycle(laydest) const;
    bool set_cycle(laydest, Cycle *);

    // modify target elements
    int set_cycles(const Slice<const Reference<Layout> > &, UpdateHint = UpdateHint());
    int get_cycles(const Slice<const Reference<Layout> > &, UpdateHint = UpdateHint());
    int clear_cycles(UpdateHint = UpdateHint()) const;

    void clear();
protected:
    Array<mapping> _bind;
};


template<typename T>
class Updates : array
{
public:
    class Element
    {
    public:
        inline Element(const T &m = T(), const UpdateHint &h = UpdateHint()) : data(m), hint(h), used(1)
        { }
        T data;
        UpdateHint hint;
        uint32_t used;
    };
    inline Slice<const Element> elements() const
    { return Slice<const Element>((const Element *) base(), length()); }
    
    inline size_t length() const
    { return array::length() / sizeof(Element); }
    
    int add(const T &m, const UpdateHint &d = UpdateHint())
    {
        Element *cmp = (Element *) base();

        for (size_t i = 0, max = length(); i < max; ++i) {
            // different data elements
            if (m != cmp[i].data) {
                continue;
            }
            if (cmp[i].hint.merge(d)) {
                ++cmp[i].used;
                return 1;
            }
        }
        if (!(cmp = (Element *) array::append(sizeof(*cmp)))) {
            return BadOperation;
        }
        new (cmp) Element(m, d);
        return 2;
    }

    void compress(int mask = 0) {
        Element *u = 0, *c = (Element *) base();
        size_t len = 0;

        for (size_t i = 0, max = length(); i < max; ++i) {
            if (!c[i].used) {
                if (!u) u = c;
                continue;
            }
            for (size_t j = i+1; j < max; ++j) {
                if (!c[j].used || c[i].data != c[j].data) {
                    continue;
                }
                if (!c[i].hint.merge(c[j].hint, mask)) {
                    continue;
                }
                c[i].used += c[j].used;
                c[j].data.~T();
                c[j].used = 0;
            }
            ++len;

            if (!u) {
                continue;
            }

            *u = *c;
            c->data.~T();
            c->used = 0;

            while (++u < c) {
                if (!u->used) {
                    break;
                }
            }
        }
        Data *d;
        if ((d = _buf.pointer())) {
            d->set_length(len * sizeof(*c));
        }
    }
};

class Graphic
{
public:

    Graphic();
    virtual ~Graphic();

    // layout (de)registration
    virtual int add_layout(Layout *, bool = true);
    virtual int remove_layout(const Layout *);
    long layout_count() const;

    // create new layout
    virtual Layout *create_layout();

    // mapping helpers
    int target(laydest &, message &, size_t = 0) const;
    metatype *item(message &, size_t = 0) const;

    // untracked reference to shedule update
    virtual bool register_update(const reference *, UpdateHint = UpdateHint());

protected:
    virtual void dispatchUpdates();
    RefArray<Layout> _layouts;
};

__MPT_NAMESPACE_END

#endif // _MPT_GRAPHIC_H
