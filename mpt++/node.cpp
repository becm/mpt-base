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
const char *metatype::string() const
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
identifier &identifier::operator=(const identifier &id)
{
    mpt_identifier_copy(this, &id);
    return *this;
}
bool identifier::setName(const char *name, int nlen)
{
    return (mpt_identifier_set(this, name, nlen)) ? true : false;
}
const char *identifier::name() const
{
    return (mpt_identifier_len(this) <= 0) ? 0 : static_cast<const char *>(mpt_identifier_data(this));
}
Slice<const char> identifier::nameData() const
{
    if (mpt_identifier_len(this) <= 0) {
        return Slice<const char>(0, 0);
    }
    const char *id = (const char *) mpt_identifier_data(this);
    return Slice<const char>(id, _len);
}

// private data for node containing single reference metadata
class NodePrivate : public node
{
public:
    NodePrivate();
protected:
    friend struct node;
    int8_t _data[128 - sizeof(node)];
};

NodePrivate::NodePrivate() : node()
{
    mpt_identifier_init(&ident, sizeof(ident) + sizeof(_data));
}

node *node::create(size_t len)
{
    static const size_t preSize = sizeof(node) - sizeof(identifier);
    void *ptr;
    len += preSize;
    if (len > sizeof(node) && len <= sizeof(NodePrivate)) {
        if (!(ptr = (malloc(sizeof(NodePrivate))))) return 0;
        return new (ptr) NodePrivate();
    }
    if (!(ptr = malloc(sizeof(node)))) return 0;
    return new (ptr) node();
}

node *node::create(const char *ident, int len)
{
    size_t need;
    if (len < 0) {
        need = ident ? (len = strlen(ident)) + 1 : 0;
    } else {
        need = len + 1;
    }
    node *n;
    // identifier in extended node data only
    if (!(n = create(need))) {
        return 0;
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
