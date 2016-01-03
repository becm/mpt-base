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
    mpt::metatype *m = mpt::Metatype::create(size);
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
        void unref();
        int assign(const value *);
        int conv(int, void *);
        metatype *clone();

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
    _mpt_geninfo_init(&_info, len - sizeof(*this) + sizeof(_info));
}

void NodePrivate::Meta::unref()
{ }
int NodePrivate::Meta::assign(const value *val)
{
    if (!val) {
        static const value empty(0, 0);
        val = &empty;
    }
    return _mpt_geninfo_value(&_info, val);
}
int NodePrivate::Meta::conv(int type, void *ptr)
{
    void **dest = (void **) ptr;

    if (type & ValueConsume) {
        return BadOperation;
    }
    if (!type) {
        static const char types[] = { metatype::Type, node::Type, 's', 0 };
        if (dest) *dest = (void *) types;
        return 0;
    }
    switch (type & 0xff) {
    case metatype::Type: ptr = static_cast<metatype *>(this); break;
    case node::Type: ptr = _base; break;
    default: return _mpt_geninfo_conv(&_info, type, ptr);
    }
    if (dest) *dest = ptr;
    return type & 0xff;
}
metatype *NodePrivate::Meta::clone()
{
    return _mpt_geninfo_clone(&_info);
}

// metatype
const char *metatype::string()
{
    return mpt_meta_data(this, 0);
}
metatype *metatype::create(size_t size)
{
    return mpt_meta_new(size);
}
int metatype::assign(const value *)
{
    return BadOperation;
}
int metatype::conv(int type, void *ptr)
{
    void **dest = (void **) ptr;

    if (type & ValueConsume) {
        return BadOperation;
    }
    if (!type) {
        static const char types[] = { Type, 0 };
        if (dest) *dest = (void *) types;
        return 0;
    }
    switch (type & 0xff) {
    case Type: ptr = this;
    default: return BadType;
    }
    if (dest) *dest = ptr;
    return type & 0xff;
}
metatype *metatype::clone()
{
    return 0;
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
uintptr_t object::addref()
{
    return 0;
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


Metatype::Metatype(size_t post) : _info(0)
{
    _mpt_geninfo_init(&_info, post+sizeof(_info));
}

Metatype::~Metatype()
{ _info = 0; }

void Metatype::unref()
{
    delete this;
}

metatype *Metatype::clone()
{
    return _mpt_geninfo_clone(&_info);
}

int Metatype::assign(const value *val)
{
    if (!val) {
        static const value empty(0, 0);
        val = &empty;
    }
    return _mpt_geninfo_value(&_info, val);
}
int Metatype::conv(int type, void *ptr)
{
    void **dest = (void **) ptr;

    if (type & ValueConsume) {
        return BadOperation;
    }
    if (!type) {
        static const char types[] = { metatype::Type, 's', 0 };
        if (dest) *dest = (void *) types;
        return 0;
    }
    switch (type & 0xff) {
    case metatype::Type: ptr = static_cast<metatype *>(this); break;
    default: return _mpt_geninfo_conv(&_info, type, ptr);
    }
    if (dest) *dest = ptr;
    return type & 0xff;
}

Slice<const char> Metatype::data() const
{
    int len = _mpt_geninfo_value(const_cast<uint64_t *>(&_info), 0);
    return Slice<const char>((const char *) (&_info + 1), len);
}

Metatype *Metatype::create(size_t size)
{
    size = MPT_align(size);

    Metatype *m;
    Metatype::Small *ms;
    Metatype::Big *mb;

    if (size > sizeof(mb->data)) return 0;

    if (size <= sizeof(ms->data)) m = ms = new Metatype::Small;
    else m = mb = new Metatype::Big;

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

__MPT_NAMESPACE_END
