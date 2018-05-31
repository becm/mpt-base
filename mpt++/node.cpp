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

// private data for aligned identifier
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
    n->ident.set_name(ident, len);
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
