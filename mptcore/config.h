/*!
 * MPT core library
 *  configuration interfaces
 */

#ifndef _MPT_CONFIG_H
#define _MPT_CONFIG_H	201502

#include "core.h"

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(node);

enum MPT_ENUM(ConfigFlags) {
	MPT_ENUM(PathHasArray)  = 0x40,
	MPT_ENUM(PathSepBinary) = 0x80
};

/*! (un)structured element path */
MPT_STRUCT(path)
{
#if defined(__cplusplus)
	path(int sep = '.', int assign = '=', const char *path = 0);
	path(path const &);
	~path();
	
	void set(const char *path, int len = -1, int sep = -1, int assign = -1);
	bool append(const char *path, int len = -1);
	
	bool next(void);
	Slice<const char> data(void) const;
	
	path & operator=(path const &);
	
	int add(void);
	int del(void);
	
	bool setValid();
	inline void clearData(void)
	{ valid = 0; }
	
    protected:
	friend MPT_INTERFACE(config);
	friend class Config;
#else
# define MPT_PATH_INIT  { 0,  0, 0, 0,  0, '.', '=', 0 }
#endif
	const char *base;   /* path data */
	
	size_t      off,    /* path start offset */
	            len;    /* path data length */
	uint16_t    valid;  /* valid after path */
	
	uint8_t     first,  /* length of first element */
	            sep,    /* path separator */
	            assign, /* assign separator */
	            flags;  /* path format */
};

MPT_INTERFACE(config)
#if defined(__cplusplus)
{
public:
	virtual int unref(void) = 0;
	virtual Reference<metatype> *query(const path *) = 0;
	virtual int remove(const path *) = 0;
	
	int environ(const char *filter = "mpt_*", int sep = '_', char * const env[] = 0);
	void del(const char *path, int sep = '.', int end = 0);
	bool set(const char *path, const char *value = 0, int sep = '.');
	metatype *get(const char *path, int sep = '.', int len = -1);
#else
; MPT_INTERFACE_VPTR(config) {
	int (*unref)(MPT_INTERFACE(config) *);
	MPT_INTERFACE(metatype) **(*query)(MPT_INTERFACE(config) *, const MPT_STRUCT(path) *);
	int (*remove)(MPT_INTERFACE(config) *, const MPT_STRUCT(path) *);
}; MPT_INTERFACE(config) {
	const MPT_INTERFACE_VPTR(config) *_vptr;
#endif
};

__MPT_EXTDECL_BEGIN

/* get/set config element */
extern MPT_INTERFACE(metatype) *mpt_config_get(MPT_INTERFACE(config) *, const char *, int __MPT_DEFPAR('.'), int __MPT_DEFPAR(0));
extern int mpt_config_set(MPT_INTERFACE(config) *, const char *, const char *, int __MPT_DEFPAR('.'), int __MPT_DEFPAR(0));
/* use config data to store environment */
extern int mpt_config_environ(MPT_INTERFACE(config) *, const char *, int __MPT_DEFPAR('_'), char * const [] __MPT_DEFPAR(0));

/* node operations for configuration */
extern MPT_STRUCT(node) *mpt_node_get(MPT_STRUCT(node) *, const MPT_STRUCT(path) *);
extern MPT_STRUCT(node) *mpt_node_query(MPT_STRUCT(node) *, MPT_STRUCT(path) *);
/* use node to store environment */
extern int mpt_node_environ(MPT_STRUCT(node) *, const char *, int __MPT_DEFPAR('_'), char * const [] __MPT_DEFPAR(0));

/* read configuration from file */
extern int mpt_config_read(MPT_STRUCT(node) *, const char *, const char * __MPT_DEFPAR(0), const char * __MPT_DEFPAR(0), MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));

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
extern int mpt_path_add(MPT_STRUCT(path) *);
extern int mpt_path_del(MPT_STRUCT(path) *);

/* remove data after path */
extern int mpt_path_invalidate(MPT_STRUCT(path) *);

/* add/delete character to/from path */
extern int mpt_path_addchar(MPT_STRUCT(path) *, int);
extern int mpt_path_delchar(MPT_STRUCT(path) *);
/* reserve data on path */
extern void *mpt_path_append(MPT_STRUCT(path) *, size_t);
/* set valid length */
extern int mpt_path_valid(MPT_STRUCT(path) *);

#ifdef _STDIO_H
/* print path to standard stream */
extern int mpt_path_fputs(const MPT_STRUCT(path) *, FILE *, const char *, const char *);
#endif

__MPT_EXTDECL_END

#if defined(__cplusplus)
class Config : public config
{
public:
    Config(const char *root = 0, int sep = '.');
    virtual ~Config();

    int unref(void);
    Reference<metatype> *query(const path *);
    int remove(const path *);

protected:
    node *_last;
private:
    node *_root;
    bool _local;
};
#endif

__MPT_NAMESPACE_END

#endif /* _MPT_CONFIG_H */
