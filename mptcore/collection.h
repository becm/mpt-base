/*!
 * MPT core library
 *  collection interfaces
 */

#ifndef _MPT_COLLECTION_H
#define _MPT_COLLECTION_H  @INTERFACE_VERSION@

#ifdef __cplusplus
# include "output.h"
#else
# include "core.h"
#endif

__MPT_NAMESPACE_BEGIN

MPT_INTERFACE(metatype);

MPT_STRUCT(node);

MPT_INTERFACE(collection);
typedef int MPT_TYPE(item_handler)(void *, const MPT_STRUCT(identifier) *, MPT_INTERFACE(metatype) *, const MPT_INTERFACE(collection) *);

#ifdef __cplusplus
MPT_INTERFACE(collection)
{
protected:
	inline ~collection() { }
public:
	virtual int each(item_handler_t *, void *) const = 0;
	virtual unsigned long clear(const metatype * = 0) = 0;
};
template<> inline __MPT_CONST_TYPE int type_properties<collection *>::id(bool) {
	return TypeCollectionPtr;
}
template <> inline const struct type_traits *type_properties<collection *>::traits() {
	return type_traits::get(id(true));
}
#else
MPT_INTERFACE_VPTR(collection) {
	int (*each)(const MPT_INTERFACE(collection) *, MPT_TYPE(item_handler), void *);
	unsigned long (*clear)(MPT_INTERFACE(collection) *, const MPT_INTERFACE(metatype) *);
}; MPT_INTERFACE(collection) {
	const MPT_INTERFACE_VPTR(collection) *_vptr;
};
#endif

#ifdef __cplusplus
/*! interface to generic groups of metatypes elements */
class group : public collection
{
protected:
	inline ~group() {}
public:
	virtual int append(const identifier *, metatype *) = 0;
	virtual metatype *create(const char *, int = -1);
	virtual int bind(const relation *, logger * = logger::default_instance());
	
	static const named_traits *get_traits();
};

/*! Relation implemetation using Group as current element */
class collection_relation : public relation
{
public:
	inline collection_relation(const collection &c, const relation *p = 0, char sep = '.') : relation(p), _curr(c), _sep(sep)
	{ }
	virtual ~collection_relation()
	{ }
	metatype *find(int type, const char *, int = -1) const __MPT_OVERRIDE;
protected:
	const collection &_curr;
	char _sep;
};

/*! Relation implemetation using node as current element */
class node_relation : public relation
{
public:
    inline node_relation(const node *n, const relation *p = 0) : relation(p), _curr(n)
    { }
    metatype *find(int type, const char *, int = -1) const;
protected:
    const node *_curr;
};

bool add_items(metatype &, const node *head, const relation *from = 0, logger * = logger::default_instance());

#endif /* __cplusplus */

__MPT_NAMESPACE_END

#endif /* _MPT_COLLECTION_H */
