/*!
 * MPT core library
 *  configuration interfaces
 */

#ifndef _MPT_CONFIG_H
#define _MPT_CONFIG_H  @INTERFACE_VERSION@

#ifdef __cplusplus
# include "types.h"
# include "array.h"
# include "output.h"
#else
# include "core.h"
#endif

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(node);
MPT_STRUCT(array);
MPT_STRUCT(value);
MPT_STRUCT(message);

MPT_INTERFACE(iterator);
MPT_INTERFACE(collection);
MPT_INTERFACE(reply_context);

typedef int MPT_TYPE(config_handler)(void *, MPT_INTERFACE(convertable) *, const MPT_INTERFACE(collection) *);

/*! (un)structured element path */
#if defined(__cplusplus)
MPT_STRUCT(path)
{
# define MPT_PATHFLAG(x) x
#else
# define MPT_PATHFLAG(x) MPT_ENUM(Path_##x)
#endif
enum MPT_PATHFLAG(Flags) {
	MPT_PATHFLAG(KeepPost)  = 0x01,
	MPT_PATHFLAG(HasArray)  = 0x40,
	MPT_PATHFLAG(SepBinary) = 0x80
};
#if defined(__cplusplus)
	path(const char * = 0, int = '.', int = 0);
	path(path const &);
	~path();
	
	void set(const char *path, int len = -1, int sep = -1, int assign = -1);
	
	bool next();
	
	bool empty() const;
	span<const char> data() const;
	span<const char> value() const;
	
	path & operator=(path const &);
	
	int add(int = -1);
	int del();
	
	bool clear_data();
	
protected:
	array::content *array_content() const;
#else
MPT_STRUCT(path)
{
# define MPT_PATH_INIT  { 0,  0, 0,  0, 0,  '.', 0 }
#endif
	const char *base;   /* path data */
	
	size_t      off,    /* path start offset */
	            len;    /* path data length */
	
	uint8_t     first,  /* length of first element */
	            flags;  /* path format */
	char        sep,    /* path separator */
	            assign; /* assign separator */
};

#if defined(__cplusplus)
MPT_INTERFACE(config)
{
protected:
	inline ~config() { }
	int get(const path &, type_t, void *) const;
public:
	int environ(const char *match = "mpt_*", int sep = '_', char * const env[] = 0);
	void del(const char *path, int sep = '.', int len = -1);
	bool set(const char *path, const char *value = 0, int sep = '.');
	
	template<typename T>
	inline bool get(const char *p, T &v) const {
		return get(path(p), v);
	}
	template<typename T>
	inline bool get(const path &p, T &v) const {
		int t = type_properties<T>::id(true);
		return t > 0 && get(p, static_cast<type_t>(t), &v) >= 0;
	}
	
	static metatype *global(const path * = 0);
	static const struct named_traits *pointer_traits();
	
	virtual int query(const path *, config_handler_t , void *) const = 0;
	virtual int assign(const path *, const value * = 0) = 0;
	virtual int remove(const path *) = 0;
	
	class root;
};
template<> inline __MPT_CONST_TYPE int type_properties<config *>::id(bool) {
	return TypeConfigPtr;
}
template <> inline const struct type_traits *type_properties<config *>::traits() {
	static const struct type_traits *traits = 0;
	return traits ? traits : (traits = type_traits::get(id(true)));
}
#else
MPT_INTERFACE(config);
MPT_INTERFACE_VPTR(config) {
	int (*query)(const MPT_INTERFACE(config) *, const MPT_STRUCT(path) *, MPT_TYPE(config_handler), void *);
	int (*assign)(MPT_INTERFACE(config) *, const MPT_STRUCT(path) *, const MPT_STRUCT(value) *);
	int (*remove)(MPT_INTERFACE(config) *, const MPT_STRUCT(path) *);
}; MPT_INTERFACE(config) {
	const MPT_INTERFACE_VPTR(config) *_vptr;
};
#endif

#if defined(__cplusplus)
MPT_STRUCT(config_item) : public unique_array<config_item>, public reference<metatype>, public identifier
{
public:
	class subtree;
	inline bool unused()
	{
		return _len == 0;
	}
};
/* type properties explicitly defined by C/C++ library methods */
template<> inline int type_properties<config_item>::id(bool);
template <> inline const struct type_traits *type_properties<config_item>::traits();
#else
MPT_STRUCT(config_item)
# ifdef _MPT_ARRAY_H
{
	_MPT_UARRAY_TYPE(MPT_STRUCT(config_item)) elements;
	MPT_INTERFACE(metatype) *value;
	MPT_STRUCT(identifier) identifier;
}
# endif /* _MPT_ARRAY_H */
;
#endif /* __cplusplus */


__MPT_EXTDECL_BEGIN

/* assign config from saved arguments */
extern int mpt_client_config(MPT_INTERFACE(config) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(0));

/* apply method to entry */
extern int mpt_config_query(const MPT_INTERFACE(config) *, const MPT_STRUCT(path) *, MPT_TYPE(config_handler), void *);
/* get config value */
extern int mpt_config_getp(const MPT_INTERFACE(config) *, const MPT_STRUCT(path) *, MPT_TYPE(type) , void * __MPT_DEFPAR(0));
extern int mpt_config_get(const MPT_INTERFACE(config) *, const char *, MPT_TYPE(type) , void * __MPT_DEFPAR(0));
/* set config value */
extern int mpt_config_set(MPT_INTERFACE(config) *, const char *, const char *, int __MPT_DEFPAR('.'), int __MPT_DEFPAR(0));
/* use config data to store environment */
extern int mpt_config_environ(MPT_INTERFACE(config) *, const char *, int __MPT_DEFPAR('_'), char * const [] __MPT_DEFPAR(0));

/* set configuration from arguments */
extern int mpt_config_args (MPT_INTERFACE(config) *, MPT_INTERFACE(iterator) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(0));
extern int mpt_config_clear(MPT_INTERFACE(config) *, MPT_INTERFACE(iterator) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(0));

/* load configuration from in (alternate) root */
extern int mpt_config_load(MPT_INTERFACE(config) *, const char *__MPT_DEFPAR(0), MPT_INTERFACE(logger) *__MPT_DEFPAR(0));

/* get config of global (sub-)tree */
extern MPT_INTERFACE(metatype) *mpt_config_global(const MPT_STRUCT(path) *);

/* get log target from config */
extern MPT_INTERFACE(logger) *mpt_config_logger(const MPT_INTERFACE(config) *);

/* get or create node for path */
extern MPT_STRUCT(node) *mpt_node_query(const MPT_STRUCT(node) *, MPT_STRUCT(path) *);

/* set path data */
extern int mpt_path_set(MPT_STRUCT(path) *, const char *, int __MPT_DEFPAR(-1));

/* clear path memory */
extern void mpt_path_fini(MPT_STRUCT(path) *);

/* save change path start to next/last element */
extern int mpt_path_next(MPT_STRUCT(path) *);
extern int mpt_path_last(MPT_STRUCT(path) *);

/* get data part of path */
extern const char *mpt_path_data(const MPT_STRUCT(path) *);

/* add/delete element to/from path map */
extern int mpt_path_add(MPT_STRUCT(path) *, int __MPT_DEFPAR(-1));
extern int mpt_path_del(MPT_STRUCT(path) *);

/* remove data after path */
extern int mpt_path_invalidate(MPT_STRUCT(path) *);

/* add/delete character to/from path */
extern int mpt_path_addchar(MPT_STRUCT(path) *, int);
extern int mpt_path_delchar(MPT_STRUCT(path) *);
/* get valid path length */
extern int mpt_path_valid(MPT_STRUCT(path) *);

#if defined(_STDIO_H) || defined(_STDIO_H_)
/* print path to standard stream */
extern int mpt_path_fputs(const MPT_STRUCT(path) *, FILE *, const char *);
#endif

/* process config element */
extern int mpt_message_assign(const MPT_STRUCT(message) *, int , int (*)(void *, const MPT_STRUCT(path) *, const MPT_STRUCT(value) *), void *);
/* query config path element */
extern int mpt_config_message_next(MPT_STRUCT(path) *, int , MPT_STRUCT(message) *);
/* query config data */
extern int mpt_config_reply(MPT_INTERFACE(reply_context) *, const MPT_INTERFACE(config) *, int , const MPT_STRUCT(message) *);

/* type traits for config item */
extern const MPT_STRUCT(type_traits) *mpt_config_item_traits();
#ifdef _MPT_ARRAY_H
/* get config item for path */
extern MPT_STRUCT(config_item) *mpt_config_item_query(const _MPT_UARRAY_TYPE(MPT_STRUCT(config_item)) *, MPT_STRUCT(path) *);
/* reserve config item for path */
extern MPT_STRUCT(config_item) *mpt_config_item_reserve(_MPT_UARRAY_TYPE(MPT_STRUCT(config_item)) *, MPT_STRUCT(path) *);
#endif

/* assign value in node tree (top level is list) */
extern MPT_STRUCT(node) *mpt_node_assign(MPT_STRUCT(node) **, const MPT_STRUCT(path) *, const MPT_STRUCT(value) *);

__MPT_EXTDECL_END

#if defined(__cplusplus)
inline bool path::empty() const
{
	return len == 0;
}
inline span<const char> path::value() const
{
	return span<const char>(base + off, len);
}
/* config with private element store */
class config::root : public config
{
public:
	root();
	virtual ~root();
	
	int assign(const path *, const value * = 0) __MPT_OVERRIDE;
	int remove(const path *) __MPT_OVERRIDE;
	int query(const path *, config_handler_t, void *) const __MPT_OVERRIDE;
	
	inline span<const config_item> items() const
	{
		return _sub.elements();
	}
protected:
	unique_array<config_item> _sub;
};

# if defined(_MPT_COLLECTION_H)
class config_item::subtree : public collection
{
public:
	subtree(const span<const config_item> &v) : _elem(v)
	{ }
	int each(item_handler_t handler, void *ctx) const __MPT_OVERRIDE
	{
		for (const config_item *e = _elem.begin(), *to = _elem.end(); e != to; ++e) {
			subtree sub(e->elements());
			int ret = handler(ctx, e, e->instance(), &sub);
			if (ret < 0) {
				return ret;
			}
		}
		return 0;
	}
private:
	const span<const config_item> _elem;
};
# endif
#endif

__MPT_NAMESPACE_END

#endif /* _MPT_CONFIG_H */
