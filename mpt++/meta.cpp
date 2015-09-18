/*
 * MPT C++ core implementation
 */

#include <string.h>
#include <stdarg.h>
#include <limits.h>

#include <sys/uio.h>

#include "queue.h"
#include "array.h"
#include "node.h"

#include "object.h"

// buffer metatype override
extern "C" mpt::metatype *mpt_meta_buffer(size_t size, const void *base)
{
    mpt::Buffer *b = new mpt::Buffer;
    if (!b) return 0;
    if (base) {
        b->write(1, base, size);
    } else {
        b->write(size, 0, 0);
    }
    return b;
}
// metatype creator override
extern "C" mpt::metatype *mpt_meta_new(size_t size)
{
    mpt::metatype *m = mpt::MetatypeGeneric::create(size);
    if (m) return m;
    mpt::Buffer *b = new mpt::Buffer;
    if (b) b->write(size, 0, 0);
    return b;
}
extern "C" mpt::node *mpt_node_new(size_t nlen, size_t dlen)
{ return mpt::node::create(nlen, dlen); }

__MPT_NAMESPACE_BEGIN

// private data for node containing single reference metadata
class NodePrivate : public node
{
public:
    NodePrivate(size_t);
    ~NodePrivate();

protected:
    class Meta : public metatype
    {
    public:
        Meta(size_t, node *);
        metatype *addref();
        int unref();
        int property(struct property *, source *);
        void *typecast(int);

    protected:
        node *_base;
        uint64_t _info;
        inline ~Meta() { }
    };
    friend struct node;
    int8_t data[128-sizeof(node)];
};

NodePrivate::NodePrivate(size_t len) : node()
{
    size_t skip = mpt_identifier_align(len);
    skip = MPT_align(skip);

    if (skip > (sizeof(ident) + sizeof(data))) {
        skip = MPT_align(sizeof(ident));
    }
    else {
        new (&ident) identifier(skip);
    }
    size_t left = sizeof(data) + sizeof(ident) - skip;
    if (left >= sizeof(Meta)) {
        _meta = new (data-sizeof(ident)+skip) Meta(left, this);
    }
}
NodePrivate::~NodePrivate()
{
    _meta->unref();
}
NodePrivate::Meta::Meta(size_t len, node *base) : _base(base)
{
    _mpt_geninfo_init(&_info, len - sizeof(*this) + sizeof(_info), 0);
}

metatype *NodePrivate::Meta::addref()
{

    metatype *meta = mpt_meta_clone(this);

    if (!(meta)) return 0;

    if ((meta = meta->addref())) {
        _base->_meta = meta;
        return meta;
    }
    meta->unref();
    return 0;
}
int NodePrivate::Meta::unref()
{
    return 0;
}
int NodePrivate::Meta::property(struct property *prop, source *src)
{
    if (prop || src) {
        return _mpt_geninfo_property(&_info, prop, src);
    }
    return 0;
}
void *NodePrivate::Meta::typecast(int t)
{
    switch (t) {
    case metatype::Type: return static_cast<metatype *>(this);
    case node::Type: return _base;
    case 's': return _mpt_geninfo_property(&_info, 0, 0) ? ((&_info) + 1) : 0;
    default: return 0;
    }
}

// metatype
const char *metatype::cast()
{
    return (const char *) mpt_meta_data(this, 0);
}

bool metatype::set(const struct property &porg, logger *out)
{
    static const char _fname[] = "mpt::metatype::set()";
    struct property pr = porg;
    int ret;
    if ((ret = mpt_meta_pset(this, &pr)) >= 0) return true;
    if (!out) return false;
    pr.name = "";
    pr.desc = 0;
    if (property(&pr) < 0) pr.name = "metatype";
    pr.val.fmt = porg.val.fmt;
    if (!(pr.val.ptr = porg.val.ptr)) { pr.val.ptr = ""; pr.val.fmt = 0; }

    if (ret < -2) {
        out->error(_fname, "%s: %s.%s", MPT_tr("bad property"), pr.name, porg.name);
    } else if (!pr.val.fmt) {
        out->error(_fname, "%s: %s.%s = \"%s\"", MPT_tr("bad property value"), pr.name, porg.name, pr.val.ptr);
    } else {
        out->error(_fname, "%s: %s.%s = <%s>", MPT_tr("bad property type"), pr.name, porg.name, pr.val.fmt);
    }
    return false;
}
metatype *metatype::create(size_t size)
{ return mpt_meta_new(size); }


struct propertyErrorOut { metatype *meta; logger *out; };
static int setProperty(void *addr, property *pr)
{
    struct propertyErrorOut *dat = (propertyErrorOut *) addr;
    if (dat->meta->set(*pr, dat->out)) return 0;
    return dat->out ? 1 : -1;
}
bool metatype::setProperties(metatype &meta, logger *out)
{
    propertyErrorOut dat = { this, out };
    return mpt_meta_foreach(&meta, setProperty, &dat);
}


MetatypeGeneric::MetatypeGeneric(size_t post, uintptr_t ref) : _info(0)
{
    _mpt_geninfo_init(&_info, post+sizeof(_info), ref);
}

MetatypeGeneric::~MetatypeGeneric()
{ _info = 0; }

int MetatypeGeneric::unref()
{
    int c = _mpt_geninfo_unref(&_info);
    if (!c) delete this;
    return c;
}

metatype *MetatypeGeneric::addref()
{ return _mpt_geninfo_addref(&_info) ? this : 0; }

int MetatypeGeneric::property(struct property *prop, source *src)
{
    return _mpt_geninfo_property(&_info, prop, src);
}
void *MetatypeGeneric::typecast(int t)
{
    switch (t) {
    case metatype::Type: return static_cast<metatype *>(this);
    case 's': return _mpt_geninfo_property(&_info, 0, 0) ? ((&_info) + 1) : 0;
    default: return 0;
    }
}

Slice<const char> MetatypeGeneric::data() const
{
    struct property pr("");

    int len = _mpt_geninfo_property(const_cast<uint64_t *>(&_info), &pr, 0);

    if (len >= 0 && !pr.val.fmt) {
        if (!len) len = pr.val.ptr ? strlen((const char *) pr.val.ptr) : 0;
        return Slice<const char>((const char *) pr.val.ptr, len);
    }
    return Slice<const char>(0, 0);
}

MetatypeGeneric *MetatypeGeneric::create(size_t size)
{
    size = MPT_align(size);

    MetatypeGeneric *m;
    MetatypeGeneric::Small *ms;
    MetatypeGeneric::Big *mb;

    if (size > sizeof(mb->data)) return 0;

    if (size <= sizeof(ms->data)) m = ms = new MetatypeGeneric::Small;
    else m = mb = new MetatypeGeneric::Big;

    return m;
}

// node
node *node::create(size_t ilen, size_t dlen)
{
    size_t isize = mpt_identifier_align(ilen);
    isize = MPT_align(isize);

    node *n;
    size_t left = sizeof(NodePrivate::data) + sizeof(n->ident) - sizeof(NodePrivate::Meta);
    if (left >= (isize + dlen)) {
        if (!(n = (node *) malloc(sizeof(NodePrivate)))) return 0;
        new (n) NodePrivate(ilen);
        if (!n->_meta) ::abort();
    }
    else if (!(n = (node *) malloc(sizeof(*n) - sizeof(n->ident) + isize))) {
        return 0;
    }
    else {
        new (n) node(dlen ? metatype::create(dlen) : 0);
        mpt_identifier_init(&n->ident, isize);
    }
    return n;
}

node *node::create(size_t dlen, const char *ident, int ilen)
{
    if (ilen < 0) {
        ilen = ident ? strlen(ident) : 0;
    }
    node *n;

    if ((n = node::create(ilen+1, dlen))) {
        n->ident.setName(ident, ilen);
    }
    return n;
}

node::~node()
{
    mpt_node_unlink(this);
    mpt_node_clear(this);
    if (_meta) _meta->unref();
}
const char *node::data(size_t *len) const
{
    if (!_meta) return 0;
    return (const char *) mpt_meta_data(_meta, len);
}

// generic class for item access
Metatype::Metatype(uintptr_t ref) : _ref(ref)
{ }
Metatype::~Metatype()
{ }

Metatype *Metatype::addref()
{
    intptr_t c = _ref;
    if (!c) return 0;
    if ((c = ++_ref) > 0) return this;  // limit to avoid race conditions
    --_ref; return 0;
}

int Metatype::unref()
{
    uintptr_t c = _ref;
    if (!c || (c = --_ref)) return c;
    delete this;
    return 0;
}

int Metatype::property(struct property *prop, source *src)
{
    if (src) {
        return -1;
    }
    if (!prop) {
        return Type;
    }
    if (!prop->name) {
        return -1;
    }
    if (*prop->name) {
        return -1;
    }
    prop->name = "store";
    prop->desc = "default data store";
    prop->val.fmt = 0;
    prop->val.ptr = 0;

    return 0;
}

__MPT_NAMESPACE_END
