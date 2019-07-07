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

MPT_INTERFACE(iterator);

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
typedef int (*MPT_TYPE(property_handler))(void *, const MPT_STRUCT(property) *);

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
	class attribute;
	
	iterator begin();
	iterator end();
	
	const_iterator const_begin() const;
	const_iterator const_end() const;
	const_iterator begin() const;
	const_iterator end() const;
	
	bool set(const char *, const value &, logger * = logger::default_instance());
	bool set(const object &, logger * = logger::default_instance());
	
	/* get property by name/position */
	attribute operator [](const char *);
	attribute operator [](int);
	
	/* get properties from node list */
	const node *set(const node *, property_handler_t , void *);
	/* set property elements */
	int properties(node **) const;
	
	/* get next node with valid/default/changed/unknown property */
	node *next_valid(const node *) const;
	node *next_default(const node *) const;
	node *next_changed(const node *) const;
	node *next_alien(const node *) const;
	
	virtual int property(struct property *) const = 0;
	virtual int set_property(const char *, convertable * = 0) = 0;
};
template<> inline __MPT_CONST_TYPE int typeinfo<object>::id()
{
	return object::Type;
}
#else
MPT_INTERFACE(object);
MPT_INTERFACE_VPTR(object) {
	int (*property)(const MPT_INTERFACE(object) *, MPT_STRUCT(property) *);
	int (*set_property)(MPT_INTERFACE(object) *, const char *, MPT_INTERFACE(convertable) *);
};MPT_INTERFACE(object) {
	const MPT_INTERFACE_VPTR(object) *_vptr;
};
#endif

__MPT_EXTDECL_BEGIN

/* get info for convertable */
extern int mpt_convertable_info(MPT_INTERFACE(convertable) *, MPT_STRUCT(property) *);

/* get object type name */
extern const char *mpt_object_typename(MPT_INTERFACE(object) *);

/* loop trough object properties */
extern int mpt_object_foreach(const MPT_INTERFACE(object) *, MPT_TYPE(property_handler) , void *, int __MPT_DEFPAR(-1));

/* set object property to match argument */
extern int mpt_object_set_iterator(MPT_INTERFACE(object) *, const char *, MPT_INTERFACE(iterator) *);
extern int mpt_object_set_string(MPT_INTERFACE(object) *, const char *, const char *, const char * __MPT_DEFPAR(0));
extern int mpt_object_set_value(MPT_INTERFACE(object) *, const char *, MPT_STRUCT(value) *);
extern int mpt_object_vset(MPT_INTERFACE(object) *, const char *, const char *, va_list);
extern int mpt_object_set (MPT_INTERFACE(object) *, const char *, const char *, ... );

/* set properties from iterator elements */
extern int mpt_object_args(MPT_INTERFACE(object) *, MPT_INTERFACE(iterator) *);

/* set properties matching node identifiers to node values */
extern int mpt_object_set_property(MPT_INTERFACE(object) *, int , const MPT_STRUCT(identifier) *, MPT_INTERFACE(convertable) *);
extern int mpt_object_set_nodes(MPT_INTERFACE(object) *, int , const MPT_STRUCT(node) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(0));

/* get matching property by name */
extern int mpt_property_match(const char *, int , const MPT_STRUCT(property) *, size_t);
/* process properties according to mask */
extern int mpt_properties_foreach(int (*)(void *, MPT_STRUCT(property) *), void *, MPT_TYPE(property_handler) , void *, int __MPT_DEFPAR(-1));

/* apply property from message text argument */
extern int mpt_message_properties(MPT_STRUCT(message) *, int , MPT_TYPE(property_handler), void *);

/* query and print all properties */
extern int mpt_properties_print(int (*)(void *, MPT_STRUCT(property) *), void *, MPT_TYPE(property_handler) , void *, int __MPT_DEFPAR(0));

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
			clear();
		}
		return *this;
	}
	inline bool operator ==(const const_iterator &cmp) const
	{
		return _pos == cmp._pos;
	}
	inline void clear()
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
			clear();
		}
		return *this;
	}
	inline bool operator ==(const iterator &cmp) const
	{
		return _pos == cmp._pos;
	}
	inline void clear()
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

class object::attribute
{
public:
	attribute(object &);
	
	inline operator const struct property&() const
	{ return _prop; }
	
	inline operator const struct value&() const
	{ return _prop.val; }
	
	bool select(const char * = 0);
	bool select(int);
	
	bool set(convertable &);
	bool set(const value &);
	
	attribute & operator= (const char *val);
	attribute & operator= (convertable &meta);
	attribute & operator= (const struct property &);
	
	inline attribute & operator= (const value &v)
	{
		if (!set(v)) _prop.name = 0;
		return *this;
	}
	template <typename T>
	attribute & operator= (const T &v)
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
private:
	attribute & operator= (const attribute &);
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
