/*!
 * MPT core library
 *  object extensions
 */

#ifndef _MPT_OBJECT_H
#define _MPT_OBJECT_H  @INTERFACE_VERSION@

#ifdef __cplusplus
# include "output.h"
#else
# include "core.h"
#endif

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(node);
MPT_STRUCT(message);

MPT_INTERFACE(metatype);

/*! single property information */
MPT_STRUCT(property)
{
#ifdef __cplusplus
public:
	enum { Type = TypeProperty };
	
	inline property(const char *n = 0, const char *v = 0) : name(n), desc(0), val(v)
	{ }
	inline property(const char *n, const uint8_t *f, const void *d) : name(n), desc(0)
	{
		val.set(f, d);
	}
	inline property(size_t pos) : name(0), desc((char *) pos)
	{ }
#else
# define MPT_PROPERTY_INIT { 0, 0, MPT_VALUE_INIT }
#endif
	const char *name;      /* property name */
	const char *desc;      /* property [index->]description */
	MPT_STRUCT(value) val; /* element value */
};
#ifdef __cplusplus
template<> inline __MPT_CONST_TYPE int typeinfo<property>::id() {
	return property::Type;
}
#endif
typedef int (*MPT_TYPE(PropertyHandler))(void *, const MPT_STRUCT(property) *);

/*! generic object interface */
#ifdef __cplusplus
MPT_INTERFACE(object)
{
protected:
	inline ~object() {}
public:
	enum { Type = TypeObject };
	
	class iterator;
	class const_iterator;
	class Property;
	
	class iterator begin();
	class iterator end();
	
	class const_iterator const_begin() const;
	class const_iterator const_end() const;
	class const_iterator begin() const;
	class const_iterator end() const;
	
	bool set(const char *, const value &, logger * = logger::defaultInstance());
	bool set(const object &, logger * = logger::defaultInstance());
	
	/* get property by name/position */
	object::Property operator [](const char *);
	object::Property operator [](int);
	
	/* get properties from node list */
	const node *set(const node *, PropertyHandler , void *);
	/* set property elements */
	int properties(node **) const;
	
	/* get next node with valid/default/changed/unknown property */
	node *next_valid(const node *) const;
	node *next_default(const node *) const;
	node *next_changed(const node *) const;
	node *next_alien(const node *) const;
	
	virtual int property_get(struct property *) const = 0;
	virtual int property_set(const char *, const metatype * = 0) = 0;
};
template<> inline __MPT_CONST_TYPE int typeinfo<object *>::id() {
	return object::Type;
}
#else
MPT_INTERFACE(object);
MPT_INTERFACE_VPTR(object) {
	int (*property_get)(const MPT_INTERFACE(object) *, MPT_STRUCT(property) *);
	int (*property_set)(MPT_INTERFACE(object) *, const char *, const MPT_INTERFACE(metatype) *);
};MPT_INTERFACE(object) {
	const MPT_INTERFACE_VPTR(object) *_vptr;
};
#endif

__MPT_EXTDECL_BEGIN

/* get object type name */
extern const char *mpt_object_typename(MPT_INTERFACE(object) *);

/* loop trough object properties */
extern int mpt_object_foreach(const MPT_INTERFACE(object) *, MPT_TYPE(PropertyHandler) , void *, int __MPT_DEFPAR(-1));

/* set object property to match argument */
extern int mpt_object_set_iterator(MPT_INTERFACE(object) *, const char *, MPT_INTERFACE(iterator) *);
extern int mpt_object_set_string(MPT_INTERFACE(object) *, const char *, const char *, const char * __MPT_DEFPAR(0));
extern int mpt_object_set_value(MPT_INTERFACE(object) *, const char *, MPT_STRUCT(value) *);
extern int mpt_object_vset(MPT_INTERFACE(object) *, const char *, const char *, va_list);
extern int mpt_object_set (MPT_INTERFACE(object) *, const char *, const char *, ... );

/* set properties from iterator elements */
extern int mpt_object_args(MPT_INTERFACE(object) *, MPT_INTERFACE(iterator) *);

/* set properties matching node identifiers to node values */
extern int mpt_object_set_property(MPT_INTERFACE(object) *, int , const MPT_STRUCT(identifier) *, const MPT_INTERFACE(metatype) *);
extern int mpt_object_set_nodes(MPT_INTERFACE(object) *, int , const MPT_STRUCT(node) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(0));

/* get matching property by name */
extern int mpt_property_match(const char *, int , const MPT_STRUCT(property) *, size_t);
/* process properties according to mask */
extern int mpt_properties_foreach(int (*)(void *, MPT_STRUCT(property) *), void *, MPT_TYPE(PropertyHandler) , void *, int __MPT_DEFPAR(-1));

/* apply property from message text argument */
extern int mpt_message_properties(MPT_STRUCT(message) *, int , MPT_TYPE(PropertyHandler), void *);

/* query and print all properties */
extern int mpt_properties_print(int (*)(void *, MPT_STRUCT(property) *), void *, MPT_TYPE(PropertyHandler) , void *, int __MPT_DEFPAR(0));

__MPT_EXTDECL_END

#ifdef __cplusplus
class object::const_iterator
{
public:
	inline const_iterator(const class object &obj) : _ref(obj), _pos(-1)
	{ }
	
	inline const_iterator &operator ++()
	{
		if (_pos >= 0 && !select(++_pos)) {
			setInvalid();
		}
		return *this;
	}
	inline bool operator ==(const const_iterator &cmp) const
	{
		return _pos == cmp._pos;
	}
	inline void setInvalid()
	{
		_prop.name = 0;
		_pos = -1;
	}
	inline const char *name()
	{
		return _prop.name;
	}
	inline const struct property &operator *() const
	{
		return _prop;
	}
	bool select(uintptr_t);
protected:
	const class object &_ref;
	intptr_t _pos;
	struct ::mpt::property _prop;
};

class object::iterator
{
public:
	inline iterator(class object &obj) : _ref(obj), _pos(-1), _name(0)
	{ }
	
	inline iterator &operator ++()
	{
		if (_pos >= 0 && !select(++_pos)) {
			setInvalid();
		}
		return *this;
	}
	inline bool operator ==(const iterator &cmp) const
	{
		return _pos == cmp._pos;
	}
	inline void setInvalid()
	{
		_name = 0;
		_pos = -1;
	}
	inline const char *name()
	{
		return _name;
	}
	inline const char *operator *() const
	{
		return _name;
	}
	bool select(uintptr_t);
protected:
	const class object &_ref;
	intptr_t _pos;
	const char *_name;
};

inline object::iterator object::begin()
{
	iterator it(*this);
	it.select(0);
	return it;
}
inline object::iterator object::end()
{
	return iterator(*this);
}

inline object::const_iterator object::const_begin() const
{
	const_iterator it(*this);
	it.select(0);
	return it;
}
inline object::const_iterator object::const_end() const
{
	return const_iterator(*this);
}
inline object::const_iterator object::begin() const
{
	return const_begin();
}
inline object::const_iterator object::end() const
{
	return const_end();
}

class object::Property
{
public:
	Property(object &);
	
	inline operator const struct property&() const
	{ return _prop; }
	
	inline operator const struct value&() const
	{ return _prop.val; }
	
	bool select(const char * = 0);
	bool select(int);
	
	bool set(const metatype &);
	bool set(const struct value &);
	
	Property & operator= (const char *val);
	Property & operator= (const metatype &meta);
	Property & operator= (const struct property &);
	
	inline Property & operator= (const value &v)
	{ if (!set(v)) _prop.name = 0; return *this; }
	
	template <typename T>
	Property & operator= (const T &v)
	{
		value::format fmt;
		value val;
		fmt.set(typeinfo<T>::id());
		val.set(fmt, &v);
		if (!set(val)) {
			_prop.name = 0;
		}
		return *this;
	}
protected:
	struct property _prop;
	object &_obj;
	friend class Object;
private:
	Property & operator= (const Property &);
};

struct node;
/*! interface to generic groups of metatypes elements */
class Group : public object
{
public:
	int property_get(struct property *) const __MPT_OVERRIDE;
	int property_set(const char *, const metatype *) __MPT_OVERRIDE;
	
	virtual const Item<metatype> *item(size_t pos) const;
	virtual Item<metatype> *append(metatype *);
	virtual size_t clear(const reference * = 0);
	virtual bool bind(const Relation &from, logger * = logger::defaultInstance());
	
	bool addItems(node *head, const Relation *from = 0, logger * = logger::defaultInstance());
protected:
	inline ~Group() {}
	virtual metatype *create(const char *, int = -1);
};

/*! Relation implemetation using Group as current element */
class GroupRelation : public Relation
{
public:
	inline GroupRelation(const Group &g, const Relation *p = 0, char sep = '.') : Relation(p), _curr(g), _sep(sep)
	{ }
	virtual ~GroupRelation()
	{ }
	metatype *find(int type, const char *, int = -1) const __MPT_OVERRIDE;
protected:
	const Group &_curr;
	char _sep;
};
#endif /* C++ */

__MPT_NAMESPACE_END

#ifdef __cplusplus
inline bool operator !=(const mpt::object::iterator &i1, const mpt::object::iterator &i2)
{
	return !(i1 == i2);
}
inline bool operator !=(const mpt::object::const_iterator &i1, const mpt::object::const_iterator &i2)
{
	return !(i1 == i2);
}

#endif /* C++ */

#endif /* MPT_OBJECT_H */
