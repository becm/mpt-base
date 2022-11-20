/*!
 * MPT core library
 *  collection interfaces
 */

#ifndef _MPT_COLLECTION_H
#define _MPT_COLLECTION_H  @INTERFACE_VERSION@

#ifdef __cplusplus
# include "output.h"
# include "meta.h"
#else
# include "core.h"
#endif

__MPT_NAMESPACE_BEGIN

MPT_INTERFACE(collection);
typedef int MPT_TYPE(item_handler)(void *, const MPT_STRUCT(identifier) *, MPT_INTERFACE(convertable) *, const MPT_INTERFACE(collection) *);

#ifdef __cplusplus
MPT_INTERFACE(collection)
{
protected:
	inline ~collection() { }
public:
	virtual int each(item_handler_t *, void *) const = 0;
	
	class relation;
	
	static const struct named_traits *pointer_traits();
};
template<> inline __MPT_CONST_TYPE int type_properties<collection *>::id(bool) {
	return TypeCollectionPtr;
}
template <> inline const struct type_traits *type_properties<collection *>::traits() {
	static const struct type_traits *traits = 0;
	return traits ? traits : (traits = type_traits::get(id(true)));
}
#else
MPT_INTERFACE_VPTR(collection) {
	int (*each)(const MPT_INTERFACE(collection) *, MPT_TYPE(item_handler), void *);
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
	virtual unsigned long clear(const metatype * = 0) = 0;
	virtual int append(const identifier *, metatype *) = 0;
	virtual metatype *create(const char *, int = -1) = 0;
	virtual int bind(const relation *, logger * = logger::default_instance()) = 0;
	
	static const struct named_traits *pointer_traits();
};

/*! Relation implemetation using Group as current element */
class collection::relation : public ::mpt::relation
{
public:
	inline relation(const collection &c, const ::mpt::relation *p = 0, char sep = '.') : _parent(p), _curr(c), _sep(sep)
	{ }
	virtual ~relation()
	{ }
	convertable *find(int type, const char *, int = -1) const __MPT_OVERRIDE;
protected:
	const ::mpt::relation *_parent;
	const collection &_curr;
	char _sep;
};

/*! Relation implemetation using node as current element */
class node;
class node_relation : public relation
{
public:
	inline node_relation(const node *n, const relation *p = 0) : _parent(p), _curr(n)
	{ }
	convertable *find(int type, const char *, int = -1) const;
protected:
	const relation *_parent;
	const node *_curr;
};

bool add_items(metatype &, const node *head, const relation *from = 0, logger * = logger::default_instance());

#endif /* __cplusplus */

__MPT_NAMESPACE_END

#endif /* _MPT_COLLECTION_H */
