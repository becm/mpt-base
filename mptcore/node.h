/*!
 * MPT core library
 *  tree/list operations with MPT metatype data
 */

#ifndef _MPT_NODE_H
#define _MPT_NODE_H  @INTERFACE_VERSION@

#ifdef __cplusplus
# include "meta.h"
#else
# include "core.h"
#endif

__MPT_NAMESPACE_BEGIN

/*! node structure, memory layout compatible with GNode */
MPT_STRUCT(node)
{
#if defined(__cplusplus)
    public:
	node(struct node const& node);
	node(metatype *ref = 0);
	~node();
	
	void setMeta(metatype *mt);
	struct node &operator=(const struct node & node);
	
	inline const Reference<metatype> &meta() const
	{ return *((Reference<metatype> *) &_meta); }
	
	const char *data(size_t * = 0) const;
	
	static node *create(size_t , const char * = 0, int = -1);
	static node *create(size_t , size_t);
	
	enum { Type = TypeNode };
#else
# define MPT_NODE_INIT { 0, 0, 0, 0, 0, MPT_IDENTIFIER_INIT }
#endif
	MPT_INTERFACE(metatype) *_meta;      /* element data reference */
	MPT_STRUCT(node)         *next,      /* next element */
	                         *prev,      /* previous element */
	                         *parent,    /* parent element */
	                         *children;  /* subelements */
	MPT_STRUCT(identifier)    ident;     /* node name */
};

#if defined(__cplusplus)
/*! Relation implemetation using node as current element */
class NodeRelation : public Relation
{
public:
    inline NodeRelation(const node *n, const Relation *p = 0) : Relation(p), _curr(n)
    { }
    object *find(int type, const char *, int = -1) const;
protected:
    const node *_curr;
};
#endif

typedef int (*MPT_TYPE(TraverseFcn))(MPT_STRUCT(node) *, void *, size_t);

__MPT_EXTDECL_BEGIN

/*** node/list/tree structure operations ***/

/* create new MPT storage node */
extern MPT_STRUCT(node) *mpt_node_new(size_t , size_t);

/* remove edges from and to node */
extern MPT_STRUCT(node) *mpt_node_unlink(MPT_STRUCT(node) *);
/* clear node children */
extern void mpt_node_clear(MPT_STRUCT(node) *);
/* remove unlinked node */
extern MPT_STRUCT(node) *mpt_node_destroy(MPT_STRUCT(node) *);

/* insert node after specified */
extern MPT_STRUCT(node) *mpt_gnode_after(MPT_STRUCT(node) *, MPT_STRUCT(node) *);
/* insert node before specified */
extern MPT_STRUCT(node) *mpt_gnode_before(MPT_STRUCT(node) *, MPT_STRUCT(node) *);

/* add node to list as nth element */
extern MPT_STRUCT(node) *mpt_gnode_add(MPT_STRUCT(node) *, int , MPT_STRUCT(node) *);
/* add node to list as nth element of identifier */
extern MPT_STRUCT(node) *mpt_node_add(MPT_STRUCT(node) *, int , MPT_STRUCT(node) *);

/* add node as nth child */
extern int mpt_gnode_insert(MPT_STRUCT(node) *, int , MPT_STRUCT(node) *);
/* add node as nth child of identifier */
extern int mpt_node_insert(MPT_STRUCT(node) *, int , MPT_STRUCT(node) *);

/* new node with identical attributes */
extern MPT_STRUCT(node) *mpt_node_clone(const MPT_STRUCT(node) *);

/* move non-existing nodes */
size_t mpt_node_move(MPT_STRUCT(node) **, MPT_STRUCT(node) *);

/* clone list/children */
extern MPT_STRUCT(node) *mpt_list_clone(const MPT_STRUCT(node) *);
extern MPT_STRUCT(node) *mpt_tree_clone(const MPT_STRUCT(node) *);


/*** list/tree search operations ***/

/* get node on position relative to node or list end */
extern MPT_STRUCT(node) *mpt_gnode_pos(const MPT_STRUCT(node) *, int);

/* get node with same identifier relative to current node */
extern MPT_STRUCT(node) *mpt_node_locate(const MPT_STRUCT(node) *, int , const void *, int);
extern MPT_STRUCT(node) *mpt_node_next(const MPT_STRUCT(node) *, const char *);

/* find node with ascii identifier in sublevel */
extern MPT_STRUCT(node) *mpt_node_find(const MPT_STRUCT(node) *, const char *, int);

/* process properties from description list */
extern const MPT_STRUCT(node) *mpt_node_foreach(const MPT_STRUCT(node) *, MPT_TYPE(PropertyHandler) , void *, int);


/* get node identifier */
extern const char *mpt_node_ident(const MPT_STRUCT(node) *);
extern const char *mpt_node_data(const MPT_STRUCT(node) *, size_t *);
/* set (zero-terminated string) node data */
extern int mpt_node_set(MPT_STRUCT(node) *, const char *);

/* restore links of node (current and all children) */
extern void mpt_gnode_relink(MPT_STRUCT(node) *);


/*** extended operations ***/

/* swap child elements */
extern void mpt_gnode_swap(MPT_STRUCT(node) *, MPT_STRUCT(node) *);
/* switch tree nodes */
extern void mpt_gnode_switch(MPT_STRUCT(node) *, MPT_STRUCT(node) *);

/* next node on same level in current tree with max. upper level search */
extern MPT_STRUCT(node) *mpt_gnode_samelevel(MPT_STRUCT(node) *, size_t );
/* first node in next sublevel in current tree with max. upper level search */
extern MPT_STRUCT(node) *mpt_gnode_sublevel(MPT_STRUCT(node) *, size_t );

/* traverse nodes according to parameters and apply function */
extern MPT_STRUCT(node) *mpt_gnode_traverse(MPT_STRUCT(node) *, int , MPT_TYPE(TraverseFcn) , void *);

/* traverse nodes tree in post/in/pre order */
extern MPT_STRUCT(node) *_mpt_gnode_traverse_in(MPT_STRUCT(node) *, int , size_t , MPT_TYPE(TraverseFcn) , void *);
extern MPT_STRUCT(node) *_mpt_gnode_traverse_pre(MPT_STRUCT(node) *, int , size_t , MPT_TYPE(TraverseFcn) , void *);
extern MPT_STRUCT(node) *_mpt_gnode_traverse_post(MPT_STRUCT(node) *, int , size_t , MPT_TYPE(TraverseFcn) , void *);
/* traverse tree levels */
extern MPT_STRUCT(node) *_mpt_gnode_traverse_level(MPT_STRUCT(node) *, int , size_t , MPT_TYPE(TraverseFcn) , void *);

__MPT_EXTDECL_END

#ifdef __cplusplus
inline node::node(struct node const& node) : next(0), prev(0), parent(0), children(0)
{ _meta = node._meta ? node._meta->clone() : 0; }
inline node::node(metatype *ref) : _meta(ref), next(0), prev(0), parent(0), children(0)
{ }
inline node &node::operator = (const struct node & other)
{
    Reference<metatype> m(other._meta ? other._meta->clone() : 0);
    if (_meta) _meta->unref();
    _meta = m.detach();
    return *this;
}
inline void node::setMeta(metatype *ref)
{
    if (_meta) _meta->unref();
    _meta = ref;
}
#endif
__MPT_NAMESPACE_END

#endif /* _MPT_NODE_H */
