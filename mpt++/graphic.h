/*!
 * MPT C++ library
 *  graphic output interface
 */

#ifndef _MPT_GRAPHIC_H
#define _MPT_GRAPHIC_H  @INTERFACE_VERSION@

#include "array.h"
#include "event.h"

#include "output.h"

__MPT_NAMESPACE_BEGIN

class Cycle;
class Graph;
class Layout;

struct msgdest;
struct message;
struct msgbind;
struct mapping;
struct event;

class Mapping : Map<msgdest, Reference<Cycle> >
{
public:
    int add(const msgbind &, const msgdest &, int = 0);
    int del(const msgbind *, const msgdest * = 0, int = 0) const;
    Array<msgdest> destinations(const msgbind &, int = 0) const;

    // save/load layout cycles
    bool saveCycles(int , const Layout &);
    bool loadCycles(int , const Layout &) const;
    bool saveCycles(int , int , const Graph &);
    bool loadCycles(int , int , const Graph &) const;
    void clearCycles(int = -1, int = -1, int = -1) const;

    const Reference<Cycle> &getCycle(const msgdest &dest) const;

    void clear();
protected:
    array _bind;
};

class UpdateHint
{
public:
    enum Match {
        MatchLayout = 1,
        MatchGraph = 2,
        MatchWorld = 4
    };
    UpdateHint(int = -1, int = -1, int = -1);

    bool merge(const UpdateHint &, int = 0);

    uint8_t match;
    uint8_t lay, grf, wld;
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
    inline Slice<const Element> slice() const
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
            return -1;
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
                if (!c->hint.merge(c->hint, mask)) {
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
        set(len * sizeof(*c));
    }
};

class Graphic
{
public:

    Graphic();
    virtual ~Graphic();

    // layout (de)registration
    virtual int addLayout(Layout *, bool = true);
    virtual int removeLayout(const Layout *);
    int layoutCount() const;

    // layout creation
    virtual Layout *createLayout();

    // mapping helpers
    int target(msgdest &, message &, size_t = 0) const;
    metatype *item(message &, size_t = 0) const;

    // untracked references to shedule update
    virtual bool registerUpdate(const metatype *, const UpdateHint & = UpdateHint());

protected:
    virtual void dispatchUpdates();
    RefArray<Layout> _layouts;
    UpdateHint _lastTarget;
};

__MPT_NAMESPACE_END

#endif // _MPT_GRAPHIC_H
