/*
 * MPT C++ metatype operations
 */

#include <limits>
#include <cstdlib>

#include <sys/uio.h>

#include "queue.h"
#include "array.h"
#include "convert.h"
#include "node.h"

#include "meta.h"

__MPT_NAMESPACE_BEGIN

// basic metatype
const char *metatype::string()
{
    return mpt_meta_data(this, 0);
}
int metatype::conv(int type, void *ptr) const
{
    void **dest = (void **) ptr;

    if (!type) {
        static const char types[] = { Type, 0 };
        if (dest) *dest = (void *) types;
        return 0;
    }
    if (type != Type) {
        return BadType;
    }
    if (dest) *dest = const_cast<metatype *>(this);
    return type;
}

// identifier operations
identifier::identifier(size_t total)
{
    mpt_identifier_init(this, total);
}
bool identifier::equal(const char *name, int nlen) const
{
    return mpt_identifier_compare(this, name, nlen) ? false : true;
}
bool identifier::setName(const identifier &id)
{
    return (mpt_identifier_copy(this, &id)) ? true : false;
}
bool identifier::setName(const char *name, int nlen)
{
    if (nlen < 0) {
        nlen = name ? strlen(name) : 0;
    }
    return (mpt_identifier_set(this, name, nlen)) ? true : false;
}
const char *identifier::name() const
{
    return (mpt_identifier_len(this) <= 0) ? 0 : (const char *) mpt_identifier_data(this);
}
Slice<const char> identifier::nameData() const
{
    const char *id = (const char *) mpt_identifier_data(this);
    return Slice<const char>(id, _len);
}

// private data for node containing single reference metadata
class NodePrivate : public node
{
public:
    NodePrivate(size_t);

    class Meta : public metatype
    {
    public:
        Meta(size_t, node *);
        void unref() __MPT_OVERRIDE;
        int conv(int, void *) const __MPT_OVERRIDE;
        metatype *clone() const __MPT_OVERRIDE;

        bool set(const char *, int = -1);
    protected:
        node *_base;
        inline ~Meta() { }
    };

    Meta *local() const;
protected:
    friend struct node;
    int8_t _data[128-sizeof(node)];
};

NodePrivate::NodePrivate(size_t len) : node()
{
    size_t skip = mpt_identifier_align(len);
    skip = MPT_align(skip);

    if (skip > (sizeof(ident) + sizeof(_data))) {
        skip = MPT_align(sizeof(ident));
    }
    else {
        new (&ident) identifier(skip);
    }
    size_t left = sizeof(_data) + sizeof(ident) - skip;
    if (left >= sizeof(Meta)) {
        _meta = new (_data - sizeof(ident) + skip) Meta(left, this);
    }
}
NodePrivate::Meta *NodePrivate::local() const
{
    // determine metatype data offset
    size_t pos = ident.totalSize() - sizeof(ident);
    pos = MPT_align(pos);
    // not enough space for propert locat metatype
    if (sizeof(_data) < (pos + sizeof(Meta) + sizeof(uint32_t))) {
        return 0;
    }
    return reinterpret_cast<Meta *>(const_cast<int8_t *>(_data + pos));
}

NodePrivate::Meta::Meta(size_t len, node *base) : _base(base)
{
    _mpt_geninfo_init(this + 1, len - sizeof(*this));
}

void NodePrivate::Meta::unref()
{ }
int NodePrivate::Meta::conv(int type, void *ptr) const
{
    void **dest = (void **) ptr;

    if (!type) {
        static const char types[] = { metatype::Type, node::Type, 's', 0 };
        if (dest) *dest = (void *) types;
        return 0;
    }
    switch (type) {
    case metatype::Type: ptr = static_cast<metatype *>(const_cast<Meta *>(this)); break;
    case node::Type: ptr = _base; break;
    default: return _mpt_geninfo_conv(this + 1, type, ptr);
    }
    if (dest) *dest = ptr;
    return type;
}
metatype *NodePrivate::Meta::clone() const
{
    return _mpt_geninfo_clone(this + 1);
}
bool NodePrivate::Meta::set(const char *str, int len)
{
    return _mpt_geninfo_set(this + 1, str, len) >= 0;
}

node *node::create(size_t ilen, value val)
{
    size_t isize = mpt_identifier_align(ilen);
    isize = MPT_align(isize);
    size_t dlen = 0;

    if (!val.fmt && val.ptr) {
        dlen = strlen(static_cast<const char *>(val.ptr)) + 1;
    }
    node *n;
    size_t left = sizeof(NodePrivate::_data) + sizeof(n->ident) - sizeof(NodePrivate::Meta);
    if (left >= (isize + dlen)) {
        NodePrivate *np;
        if (!(np = static_cast<NodePrivate *>(malloc(sizeof(NodePrivate))))) return 0;
        new (np) NodePrivate(ilen);
        if (dlen) {
            np->local()->set(static_cast<const char *>(val.ptr), dlen - 1);
        }
        return np;
    }
    metatype *mt;
    if (!(mt = metatype::create(val))) {
        return 0;
    }
    if (!(n = static_cast<node *>(malloc(sizeof(*n) - sizeof(n->ident) + isize)))) {
        mt->unref();
        return 0;
    }
    new (n) node(mt);
    mpt_identifier_init(&n->ident, isize);

    return n;
}

node *node::create(const char *ident, int len)
{
    size_t need;
    if (len < 0) {
        need = ident ? (len = strlen(ident)) + 1 : 0;
    } else {
        need = len + 1;
    }
    size_t max = identifier::minimalLength();
    node *n;
    // identifier in extended node data only
    if (need > max
        && (need - max) < sizeof(NodePrivate::_data)) {
        n = new NodePrivate(need);
    } else {
        n = new node();
    }
    n->ident.setName(ident, len);
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
