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
MPT_STRUCT(value);

MPT_INTERFACE(iterator);

/*! single property information */
MPT_STRUCT(property)
#ifdef _MPT_TYPES_H
{
#ifdef __cplusplus
public:
	inline property(const char *n = 0, const char *v = 0) : name(n), desc(0)
	{
		set(v);
	}
	inline property(const char *n, int t, const void *d) : name(n), desc(0)
	{
		set(t, d);
	}
	inline property(size_t pos) : name(0), desc((char *) pos)
	{ }
	
	bool set(const char *);
	int set(int , const void *);
	
	template <typename T>
	bool set(const T &val)
	{
		return set(type_properties<T>::id(true), &val);
	}
#else
# define MPT_PROPERTY_INIT { 0, 0, MPT_VALUE_INIT(0, 0), { 0 } }
# define MPT_property_set_string(p, s) ( \
	(p)->val._type = 's', \
	(p)->val._addr = (*((const char **) (p)->_buf) = (s), (p)->_buf))
# define MPT_property_set_data(p, t, d) ( \
	(p)->val._type = (t), \
	(p)->val._addr = (sizeof(*(d)) > sizeof((p)->_buf)) ? 0 : memcpy((p)->_buf, (d), sizeof(*(d))))
#endif
	const char *name;      /* property name */
	const char *desc;      /* property [index->]description */
	MPT_STRUCT(value) val; /* element value */
#ifdef __cplusplus
private:
#endif
	/* compile-time detection of sizeof(void *) */
	uint8_t _buf[UINTPTR_MAX <= UINT32_MAX ? 4 * sizeof(uint32_t) : 2 * sizeof(void*)];
}
#endif /* _MPT_TYPES_H */
;
#ifdef __cplusplus
template<> inline __MPT_CONST_TYPE int type_properties<property>::id(bool) {
	return TypeProperty;
}
template <> inline const struct type_traits *type_properties<property>::traits() {
	return type_traits::get(id(true));
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
	class iterator;
	class const_iterator;
	class attribute;
	
	iterator begin();
	iterator end();
	
	const_iterator const_begin() const;
	const_iterator const_end() const;
	const_iterator begin() const;
	const_iterator end() const;
	
	bool set(const char *, const char *, logger * = logger::default_instance());
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
	
	static const struct named_traits *pointer_traits();
};
template<> inline __MPT_CONST_TYPE int type_properties<object *>::id(bool) {
	return TypeObjectPtr;
}
template <> inline const struct type_traits *type_properties<object *>::traits() {
	return type_traits::get(id(true));
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
extern int mpt_object_set_value(MPT_INTERFACE(object) *, const char *, const MPT_STRUCT(value) *);
#ifdef _STDARG_H
extern int mpt_object_vset(MPT_INTERFACE(object) *, const char *, const char *, va_list);
#endif
extern int mpt_object_set (MPT_INTERFACE(object) *, const char *, const char *, ... );

/* set properties from iterator elements */
extern int mpt_object_args(MPT_INTERFACE(object) *, MPT_INTERFACE(iterator) *);

/* set properties matching node identifiers to node values */
extern int mpt_object_set_property(MPT_INTERFACE(object) *, int , const MPT_STRUCT(identifier) *, MPT_INTERFACE(convertable) *);
extern int mpt_object_set_nodes(MPT_INTERFACE(object) *, int , const MPT_STRUCT(node) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(0));

/* get matching property by name */
extern int mpt_property_match(const char *, int , const char * const *, size_t);
/* copy property content */
extern void mpt_property_copy(MPT_STRUCT(property) *, const MPT_STRUCT(property) *);
/* process properties according to mask */
extern int mpt_properties_foreach(int (*)(void *, MPT_STRUCT(property) *), void *, MPT_TYPE(property_handler) , void *, int __MPT_DEFPAR(-1));

/* apply property from message text argument */
extern int mpt_message_properties(MPT_STRUCT(message) *, int , MPT_TYPE(property_handler), void *);

/* query and print all properties */
extern int mpt_properties_print(int (*)(void *, MPT_STRUCT(property) *), void *, MPT_TYPE(property_handler) , void *, int __MPT_DEFPAR(0));

__MPT_EXTDECL_END

#ifdef __cplusplus
class object::attribute
{
public:
	attribute(object &);
	attribute(const attribute &);
	
	inline operator const struct property&() const
	{ return _prop; }
	
	inline operator const struct value&() const
	{ return _prop.val; }
	
	bool select(const char * = 0);
	bool select(int);
	
	bool set(convertable &);
	bool set(const value &);
	
	attribute & operator= (const char *);
	
	inline attribute & operator= (char *v)
	{
		return operator=(static_cast<const char *>(v));
	}
	inline attribute & operator= (const value &v)
	{
		if (!set(v)) {
			_prop.name = 0;
		}
		return *this;
	}
	inline attribute & operator= (convertable &c)
	{
		if (!set(c)) {
			_prop.name = 0;
		}
		return *this;
	}
	template <typename T>
	attribute & operator= (const T &v)
	{
		int type = type_properties<T>::id(true);
		value val;
		if (!val.set(type, &v) || !set(val)) {
			_prop.name = 0;
		}
		return *this;
	}
protected:
	friend class iterator;
	struct property _prop;
	object &_obj;
private:
	attribute & operator= (const attribute &);
};

class object::const_iterator
{
public:
	inline const_iterator(const class object &obj) : _ref(obj), _pos(-1)
	{ }
	
	inline const_iterator &operator ++()
	{
		if (_pos >= 0 && !select(_pos + 1)) {
			clear();
		}
		return *this;
	}
	inline bool operator ==(const const_iterator &cmp) const
	{
		return &_ref == &cmp._ref && _pos == cmp._pos;
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
	struct property _prop;
	const object &_ref;
	intptr_t _pos;
};

class object::iterator
{
public:
	inline iterator(class object &obj) : _attr(obj), _pos(-1)
	{ }
	
	inline iterator &operator ++()
	{
		if (_pos >= 0 && !select(_pos + 1)) {
			clear();
		}
		return *this;
	}
	inline bool operator ==(const iterator &cmp) const
	{
		return &_attr._obj == &cmp._attr._obj && _pos == cmp._pos;
	}
	inline void clear()
	{
		_pos = -1;
	}
	inline const attribute &operator *() const
	{
		return _attr;
	}
	inline bool select(uintptr_t pos)
	{
		bool success = _attr.select(pos);
		if (success) {
			_pos = pos;
		}
		return success;
	}
protected:
	attribute _attr;
	intptr_t _pos;
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
#endif /* C++ */

__MPT_NAMESPACE_END

#ifdef __cplusplus
std::ostream &operator<<(std::ostream &, const mpt::object &);

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
