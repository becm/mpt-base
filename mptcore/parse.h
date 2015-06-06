/*!
 * MPT core library
 *  simple parser and configuration
 */

#ifndef _MPT_PARSE_H
#define _MPT_PARSE_H	201502

#include <ctype.h>
#include <limits.h>

#include "core.h"

struct iovec;

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(node);
MPT_STRUCT(path);

enum MPT_ENUM(ParseFlags) {
	MPT_ENUM(ParseSection)  = 0x1,
	MPT_ENUM(ParseSectEnd)  = 0x2,
	MPT_ENUM(ParseOption)   = 0x3,
	MPT_ENUM(ParseData)     = 0x4,
	MPT_ENUM(ParseInternal) = 0x8,
	
	MPT_ENUM(ParseSectName) = 0x10,
	MPT_ENUM(ParseOptName)  = 0x20,
	MPT_ENUM(ParseDataEnd)  = 0x40,
	
	MPT_ENUM(NameNumStart)  = 0x1,  /* allow numeric initial character */
	MPT_ENUM(NameNumCont)   = 0x2,  /* allow numeric continous character */
	MPT_ENUM(NameNumeral)   = 0x3,
	MPT_ENUM(NameSpecial)   = 0x4,  /* allow special character */
	MPT_ENUM(NameSpace)     = 0x8,  /* allow space characters */
	
	MPT_ENUM(NameEmpty)     = 0x10, /* allow empty name */
	MPT_ENUM(NameBinary)    = 0x20  /* allow binary character */
};

/* simple format description */
MPT_STRUCT(parsefmt)
{
#ifdef __cplusplus
	parsefmt(const char *fmt = 0);
	
	inline bool isComment(int c) const
	{ return c && (c==com[0] || c==com[1] || c==com[2] || c==com[3]); }
	inline bool isEscape(int c) const
	{ return c && (c==esc[0] || c==esc[1] || c==esc[2]); }
#else
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
MPT_STRUCT(parseflg)
{
	uint8_t sect,    /* section name format */
	        opt;     /* option name format */
};
/* parser input metadata */
MPT_STRUCT(parse)
{
#ifdef __cplusplus
	parse(const char *file = 0);
#endif
	struct {
	int (*getc)(void *);
	void *arg;
	} source;                    /* character source */
	size_t line;                 /* current line */
	
	struct {
	int (*ctl)(void *, const MPT_STRUCT(path) *, int);
	void *arg;
	} check;                     /* check path element before adding */
	
	MPT_STRUCT(parsefmt) format; /* parse format information */
	uint16_t             lastop; /* previous operation */
	MPT_STRUCT(parseflg) name;   /* section/option name format */
};

typedef int (*MPT_TYPE(ParserFcn))(MPT_STRUCT(parse) *, MPT_STRUCT(path) *);

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

/* initialize parser structure */
extern void mpt_parse_init(MPT_STRUCT(parse) *);

/* read character from source, save in 'post' path area */
extern int mpt_parse_getchar(MPT_STRUCT(parse) *, MPT_STRUCT(path) *);

/* continue to next visible character / end of line without saving */
extern int mpt_parse_nextvis(MPT_STRUCT(parse) *, const void *, size_t);
extern int mpt_parse_endline(MPT_STRUCT(parse) *);

/* get character from file */
extern int mpt_getchar_file(void *);
/* get character from stdio stream */
#if defined(_STDIO_H) || defined(_STDIO_H_)
extern int mpt_getchar_stdio(FILE *);
#endif
/* get character from IO vector */
extern int mpt_getchar_iovec(struct iovec *);


/* get element for prepending section names */
extern int mpt_parse_format_pre(MPT_STRUCT(parse) *, MPT_STRUCT(path) *);
/* get next element for encapsulated section names */
extern int mpt_parse_format_enc(MPT_STRUCT(parse) *, MPT_STRUCT(path) *);
/* get element for separating section names */
extern int mpt_parse_format_sep(MPT_STRUCT(parse) *, MPT_STRUCT(path) *);

/* get option element */
extern int mpt_parse_option(MPT_STRUCT(parse) *, MPT_STRUCT(path) *);
/* get data element */
extern int mpt_parse_data(MPT_STRUCT(parse) *, MPT_STRUCT(path) *);

/* insert/append elements */
extern MPT_STRUCT(node) *mpt_parse_insert(MPT_STRUCT(node) *, const MPT_STRUCT(path) *);
extern MPT_STRUCT(node) *mpt_parse_append(MPT_STRUCT(node) *, const MPT_STRUCT(path) *, int , int);


/* parse MPT configuration tree */
extern int mpt_parse_config(MPT_TYPE(ParserFcn) , MPT_STRUCT(parse) *, MPT_STRUCT(node) *);

__MPT_EXTDECL_END

#ifdef __cplusplus
class Parse
{
public:
    Parse(parse *);
    virtual ~Parse();

    virtual size_t line() const;
    virtual bool reset();
    virtual bool setFormat(const char *);
    virtual int read(struct node &, logger * = logger::defaultInstance());

protected:
    parse *_parse;
    ParserFcn _next;
};
#endif

__MPT_NAMESPACE_END

#endif /* _MPT_PARSE_H */
