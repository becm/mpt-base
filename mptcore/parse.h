/*!
 * MPT core library
 *  simple parser and configuration
 */

#ifndef _MPT_PARSE_H
#define _MPT_PARSE_H  @INTERFACE_VERSION@

#include <ctype.h>
#include <limits.h>

#include "core.h"

struct iovec;

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(node);
MPT_STRUCT(path);

/* simple format description */
MPT_STRUCT(parsefmt)
{
#ifdef __cplusplus
	parsefmt();
	
	inline bool isComment(int c) const
	{ return c && (c==com[0] || c==com[1] || c==com[2] || c==com[3]); }
	inline bool isEscape(int c) const
	{ return c && (c==esc[0] || c==esc[1] || c==esc[2]); }
#else
# define MPT_PARSEFMT_INIT  { '{', '}', 0, '=', 0, { '"', '\'' }, { '#' } }
# define MPT_iscomment(f,c) ((c) && ((c)==(f)->com[0]||(c)==(f)->com[1]||(c)==(f)->com[2]||(c)==(f)->com[3]))
# define MPT_isescape(f,c)  ((c) && ((c)==(f)->esc[0]||(c)==(f)->esc[1]||(c)==(f)->esc[2]))
#endif
	uint8_t sstart,
	        send,    /* section start/end character */
	        ostart,
	        assign,
	        oend,    /* option start/assign/end character */
	        esc[3],  /* escape characters */
	        com[4];  /* comment characters */
};
/* parser name flags */
#ifdef __cplusplus
MPT_STRUCT(parseflg)
{
# define MPT_NAMEFLAG(x) x
#else
# define MPT_NAMEFLAG(x) MPT_ENUM(Name##x)
#endif
enum MPT_NAMEFLAG(Flags) {
	MPT_NAMEFLAG(NumStart)  = 0x1,  /* allow numeric initial character */
	MPT_NAMEFLAG(NumCont)   = 0x2,  /* allow numeric continous character */
	MPT_NAMEFLAG(Numeral)   = 0x3,
	MPT_NAMEFLAG(Special)   = 0x4,  /* allow special character */
	MPT_NAMEFLAG(Space)     = 0x8,  /* allow space characters */
	
	MPT_NAMEFLAG(Empty)     = 0x10, /* allow empty name */
	MPT_NAMEFLAG(Binary)    = 0x20  /* allow binary character */
};
#ifdef __cplusplus
	inline parseflg() : sect(0xff), opt(0xff)
	{ }
#else
MPT_STRUCT(parseflg)
{
# define MPT_PARSEFLG_INIT { 0xff, 0xff }
#endif
	uint8_t sect,    /* section name format */
	        opt;     /* option name format */
};
typedef int (*MPT_TYPE(PathHandler))(void *, const MPT_STRUCT(path) *, const MPT_STRUCT(value) *, int, int);

/* parser input metadata */
MPT_STRUCT(parseinput)
{
#ifdef __cplusplus
	parseinput();
#else
# define MPT_PARSEINPUT_INIT { 0, 0, 1 }
#endif
	int (*getc)(void *); /* byte read operation */
	void *arg;           /* reader context */
	size_t line;         /* current line */
};
/* parser context */
#ifdef __cplusplus
MPT_STRUCT(parse)
{
# define MPT_PARSEFLAG(x) x
#else
# define MPT_PARSEFLAG(x) MPT_ENUM(Parse##x)
#endif
enum MPT_PARSEFLAG(Flags) {
	MPT_PARSEFLAG(Section)  = 0x1,
	MPT_PARSEFLAG(SectEnd)  = 0x2,
	MPT_PARSEFLAG(Option)   = 0x3,
	MPT_PARSEFLAG(Data)     = 0x4,
	MPT_PARSEFLAG(Name)     = 0x8
};
#ifdef __cplusplus
	inline parse() : prev(0), curr(0)
	{ }
#else
MPT_STRUCT(parse)
{
# define MPT_PARSE_INIT  { MPT_PARSEINPUT_INIT,  0,  0, 0, MPT_PARSEFLG_INIT }
#endif
	MPT_STRUCT(parseinput) src;  /* character source */
	
	uint16_t             valid;  /* valid size of post data */
	
	uint8_t              prev;   /* previous operation */
	uint8_t              curr;   /* current operation */
	MPT_STRUCT(parseflg) name;   /* section/option name format */
};
typedef int (*MPT_TYPE(ParserFcn))(void *, MPT_STRUCT(parse) *, MPT_STRUCT(path) *);

__MPT_EXTDECL_BEGIN

/* check option/section name according to limit flags */
extern int mpt_parse_ncheck(const char *, size_t , int);
/* set accept flags for name */
extern int mpt_parse_accept(MPT_STRUCT(parseflg) *, const char *);


/* correct path */
extern int mpt_parse_post(MPT_STRUCT(path) *, int);


/* decode parameter and format type */
extern int mpt_parse_format(MPT_STRUCT(parsefmt) *, const char *);
extern MPT_TYPE(ParserFcn) mpt_parse_next_fcn(int);


/* read character from source, save in 'post' path area */
extern int mpt_parse_getchar(MPT_STRUCT(parseinput) *, MPT_STRUCT(path) *);

/* continue to next visible character / end of line without saving */
extern int mpt_parse_nextvis(MPT_STRUCT(parseinput) *, const void *, size_t);
extern int mpt_parse_endline(MPT_STRUCT(parseinput) *);

/* get character from file */
extern int mpt_getchar_file(void *);
/* get character from stdio stream */
#if defined(_STDIO_H) || defined(_STDIO_H_)
extern int mpt_getchar_stdio(FILE *);
#endif
/* get character from IO vector */
extern int mpt_getchar_iovec(struct iovec *);


/* get element for prepending section names */
extern int mpt_parse_format_pre(const MPT_STRUCT(parsefmt) *, MPT_STRUCT(parse) *, MPT_STRUCT(path) *);
/* get next element for encapsulated section names */
extern int mpt_parse_format_enc(const MPT_STRUCT(parsefmt) *, MPT_STRUCT(parse) *, MPT_STRUCT(path) *);
/* get element for separating section names */
extern int mpt_parse_format_sep(const MPT_STRUCT(parsefmt) *, MPT_STRUCT(parse) *, MPT_STRUCT(path) *);

/* get option element */
extern int mpt_parse_option(const MPT_STRUCT(parsefmt) *, MPT_STRUCT(parse) *, MPT_STRUCT(path) *);
/* get data element */
extern int mpt_parse_data(const MPT_STRUCT(parsefmt) *, MPT_STRUCT(parse) *, MPT_STRUCT(path) *);

/* create/modify current node element */
extern MPT_STRUCT(node) *mpt_node_append(MPT_STRUCT(node) *, const MPT_STRUCT(path) *, const MPT_STRUCT(value) *, int , int);
/* set node elements from file */
extern int mpt_node_parse(MPT_STRUCT(node) *, const MPT_STRUCT(value) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));

/* parse configuration tree */
extern int mpt_parse_config(MPT_TYPE(ParserFcn) , void *, MPT_STRUCT(parse) *, MPT_TYPE(PathHandler), void *);
/* save config tree to node children */
extern int mpt_parse_node(MPT_STRUCT(node) *, MPT_STRUCT(parse) *, const char *);

#if _POSIX_C_SOURCE >= 200809L
/* load configuration file/path */
extern int _mpt_config_load(const char *__MPT_DEFPAR(0), MPT_INTERFACE(logger) *__MPT_DEFPAR(0), MPT_TYPE(PathHandler)__MPT_DEFPAR(0), void *__MPT_DEFPAR(0));
extern int mpt_config_load (const char *__MPT_DEFPAR(0), MPT_INTERFACE(logger) *__MPT_DEFPAR(0), const MPT_STRUCT(path) *__MPT_DEFPAR(0));
#endif

extern int mpt_string_nextvis(const char **);

__MPT_EXTDECL_END

#ifdef __cplusplus
class Parse
{
public:
    Parse();
    virtual ~Parse();
    
    virtual bool reset();
    virtual bool setFormat(const char *);
    virtual bool open(const char *);
    virtual int read(struct node &, logger * = logger::defaultInstance());
    
    inline size_t line() const
    { return _d.src.line; }
    
protected:
    parse _d;
    ParserFcn _next;
    void *_nextCtx;
};
inline bool Parse::setFormat(const char *)
{ return false; }

class LayoutParser : public Parse
{
public:
    LayoutParser();
    ~LayoutParser();
    
    bool reset() __MPT_OVERRIDE;
    bool open(const char *) __MPT_OVERRIDE;
    bool setFormat(const char *) __MPT_OVERRIDE;
    
    static const char *defaultFormat();
    
protected:
    parsefmt _fmt;
    char *_fn;
};
#endif

__MPT_NAMESPACE_END

#endif /* _MPT_PARSE_H */
