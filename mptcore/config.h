/*!
 * MPT core library
 *  configuration interfaces
 */

#ifndef _MPT_CONFIG_H
#define _MPT_CONFIG_H  @INTERFACE_VERSION@

#ifdef __cplusplus
# include "array.h"
#else
# include "core.h"
#endif

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(node);
MPT_STRUCT(array);
MPT_STRUCT(message);

MPT_INTERFACE(iterator);
MPT_INTERFACE(reply_context);

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
	Slice<const char> data() const;
	Slice<const char> value() const;
	
	path & operator=(path const &);
	
	int add(int = -1);
	int del();
	
	bool clearData();
	
protected:
	::mpt::array::Data *array() const;
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
public:
	enum { Type = TypeConfig };
	
	int environ(const char *match = "mpt_*", int sep = '_', char * const env[] = 0);
	void del(const char *path, int sep = '.', int len = -1);
	bool set(const char *path, const char *value = 0, int sep = '.');
	const metatype *get(const char *path, int sep = '.', int len = -1);
	
	static metatype *global(const path * = 0);
	
	virtual const metatype *query(const path *) const = 0;
	virtual int assign(const path *, const value * = 0) = 0;
	virtual int remove(const path *) = 0;
};
template<> inline __MPT_CONST_EXPR int typeIdentifier<config>() { return config::Type; }
#else
MPT_INTERFACE(config);
MPT_INTERFACE_VPTR(config) {
	const MPT_INTERFACE(metatype) *(*query)(const MPT_INTERFACE(config) *, const MPT_STRUCT(path) *);
	int (*assign)(MPT_INTERFACE(config) *, const MPT_STRUCT(path) *, const MPT_STRUCT(value) *);
	int (*remove)(MPT_INTERFACE(config) *, const MPT_STRUCT(path) *);
}; MPT_INTERFACE(config) {
	const MPT_INTERFACE_VPTR(config) *_vptr;
};
#endif

__MPT_EXTDECL_BEGIN

/* assign config from saved arguments */
extern int mpt_client_config(MPT_INTERFACE(config) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(0));

/* get/set config element */
extern const MPT_INTERFACE(metatype) *mpt_config_get(const MPT_INTERFACE(config) *, const char *, int __MPT_DEFPAR('.'), int __MPT_DEFPAR(0));
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
extern MPT_STRUCT(node) *mpt_node_query(MPT_STRUCT(node) *, MPT_STRUCT(path) *, const MPT_STRUCT(value) *);

/* set path data */
extern size_t mpt_path_set(MPT_STRUCT(path) *, const char *, int __MPT_DEFPAR(-1));

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
/* query config element */
extern const MPT_INTERFACE(metatype) *mpt_config_message_next(const MPT_INTERFACE(config) *, int , MPT_STRUCT(message) *);
/* query config data */
extern int mpt_config_reply(MPT_INTERFACE(reply_context) *, const MPT_INTERFACE(config) *, int , const MPT_STRUCT(message) *);

/* assign value in node tree (top level is list) */
extern MPT_STRUCT(node) *mpt_node_assign(MPT_STRUCT(node) **, const MPT_STRUCT(path) *, const MPT_STRUCT(value) *);

__MPT_EXTDECL_END

#if defined(__cplusplus)
inline bool path::empty() const
{
	return len == 0;
}
inline Slice<const char> path::value() const
{
	return Slice<const char>(base + off, len);
}
/* config with private element store */
class Config : public config
{
public:
	Config();
	virtual ~Config();
	
	const metatype *query(const path *) const __MPT_OVERRIDE;
	int assign(const path *, const value * = 0) __MPT_OVERRIDE;
	int remove(const path *) __MPT_OVERRIDE;
	
	class Element;
	inline Slice<const Element> elements() const
	{
		return _sub.slice();
	}
protected:
	static Element *getElement(const UniqueArray<Element> &, path &);
	static Element *makeElement(UniqueArray<Element> &, path &);
	UniqueArray<Element> _sub;
};
class Config::Element : public UniqueArray<Config::Element>, public Item<metatype>
{
public:
	inline bool unused()
	{
		return _len == 0;
	}
private:
	Element & operator =(const Element &from);
};
#endif

__MPT_NAMESPACE_END

#endif /* _MPT_CONFIG_H */
