/*
 * MPT C++ core implementation
 */

#include <limits>
#include <cstdlib>

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
        int assign(const value *);
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

    metatype *ref, *meta;

    if (!(meta = mpt_meta_clone(this))) {
        return 0;
    }
    if ((ref = meta->addref())) {
        _base->_meta = ref;
        return meta;
    }
    meta->unref();
    return 0;
}
int NodePrivate::Meta::unref()
{
    return 0;
}
int NodePrivate::Meta::assign(const value *val)
{
    if (!val) {
        static const value empty(0, 0);
        val = &empty;
    }
    return _mpt_geninfo_value(&_info, val);
}
void *NodePrivate::Meta::typecast(int t)
{
    switch (t) {
    case metatype::Type: return static_cast<metatype *>(this);
    case node::Type: return _base;
    case 's': return _mpt_geninfo_value(&_info, 0) ? ((&_info) + 1) : 0;
    default: return 0;
    }
}

// metatype
const char *metatype::cast()
{
    return (const char *) mpt_meta_data(this, 0);
}
metatype *metatype::create(size_t size)
{
    return mpt_meta_new(size);
}

// object
bool object::set(const char *name, const value &val, logger *out)
{
    static const char _fname[] = "mpt::object::set";
    int ret;
    if ((ret = mpt_object_pset(this, name, &val)) >= 0) return true;
    if (!out) return false;
    struct property pr;
    if (property(&pr) < 0) pr.name = "object";
    pr.val.fmt = val.fmt;
    if (!(pr.val.ptr = val.ptr)) { pr.val.ptr = ""; pr.val.fmt = 0; }

    if (ret == BadArgument) {
        out->error(_fname, "%s: %s.%s", MPT_tr("bad property"), pr.name, name);
    } else if (ret == BadValue) {
        out->error(_fname, "%s: %s.%s = \"%s\"", MPT_tr("bad property value"), pr.name, name, pr.val.ptr);
    } else if (ret == BadType) {
        out->error(_fname, "%s: %s.%s = <%s>", MPT_tr("bad property type"), pr.name, name, pr.val.fmt);
    }
    return false;
}
struct propertyErrorOut { object *obj; logger *out; };
static int objectSetProperty(void *addr, const property *pr)
{
    struct propertyErrorOut *dat = (propertyErrorOut *) addr;
    if (dat->obj->set(pr->name, pr->val, dat->out)) return 0;
    return dat->out ? 1 : -1;
}
bool object::setProperties(const object &from, logger *out)
{
    propertyErrorOut dat = { this, out };
    return mpt_object_foreach(&from, objectSetProperty, &dat);
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

int MetatypeGeneric::assign(const value *val)
{
    if (!val) {
        static const value empty(0, 0);
        val = &empty;
    }
    return _mpt_geninfo_value(&_info, val);
}
void *MetatypeGeneric::typecast(int t)
{
    switch (t) {
    case metatype::Type: return static_cast<metatype *>(this);
    case 's': return _mpt_geninfo_value(&_info, 0) ? ((&_info) + 1) : 0;
    default: return 0;
    }
}

Slice<const char> MetatypeGeneric::data() const
{
    int len = _mpt_geninfo_value(const_cast<uint64_t *>(&_info), 0);
    return Slice<const char>((const char *) (&_info + 1), len);
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
        if (!(n = (NodePrivate *) malloc(sizeof(NodePrivate)))) return 0;
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
    return _ref.raise() ? this : 0;
}
int Metatype::unref()
{
    uintptr_t c = _ref.lower();
    if (!c) delete this;
    return c;
}
int Metatype::assign(const value *val)
{
    return val ? BadArgument : 0;
}
void *Metatype::typecast(int type)
{
    if (!type) {
        static const char types[] = { metatype::Type, 0 };
        return (void *) types;
    }
    return (type == metatype::Type) ? this : 0;
}

__MPT_NAMESPACE_END
