/*!
 * MPT core library
 *  simple parser and configuration
 */

#ifndef _MPT_PARSE_H
#define _MPT_PARSE_H  @INTERFACE_VERSION@

#ifdef __cplusplus
# include "output.h"
#else
# include "core.h"
#endif

struct iovec;

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(node);
MPT_STRUCT(path);
MPT_STRUCT(value);

/* simple format description */
MPT_STRUCT(parser_format)
{
#ifdef __cplusplus
	parser_format();
	
	inline bool is_comment(int c) const
	{
		return c && (c == com[0] || c == com[1] || c == com[2] || c == com[3]);
	}
	inline bool is_escape(int c) const
	{
		return c && (c == esc[0] || c == esc[1] || c == esc[2]);
	}
#else
# define MPT_PARSER_FORMAT_INIT  { '{', '}', 0, '=', 0, { '"', '\'' }, { '#' } }
# define MPT_iscomment(f,c)      ((c) && ((c)==(f)->com[0]||(c)==(f)->com[1]||(c)==(f)->com[2]||(c)==(f)->com[3]))
# define MPT_isescape(f,c)       ((c) && ((c)==(f)->esc[0]||(c)==(f)->esc[1]||(c)==(f)->esc[2]))
#endif
	uint8_t sstart,
	        send,    /* section start/end character */
	        ostart,
	        assign,
	        oend,    /* option start/assign/end character */
	        esc[3],  /* escape characters */
	        com[4];  /* comment characters */
};
/* element name flags */
#ifdef __cplusplus
MPT_STRUCT(parser_allow)
{
# define MPT_NAMEFLAG(x) x
#else
# define MPT_NAMEFLAG(x) MPT_ENUM(Name##x)
#endif
enum MPT_NAMEFLAG(Flags) {
	MPT_NAMEFLAG(NumStart)  = 0x1,  /* 'f', allow numeric initial character */
	MPT_NAMEFLAG(NumCont)   = 0x2,  /* 'c', allow numeric continous character */
	MPT_NAMEFLAG(Numeral)   = 0x3,  /* 'n' */
	MPT_NAMEFLAG(Special)   = 0x4,  /* 's', allow special character */
	MPT_NAMEFLAG(Space)     = 0x8,  /* 'w', allow whitespace characters */
	
	MPT_NAMEFLAG(Empty)     = 0x10, /* 'e', allow empty name */
	MPT_NAMEFLAG(Binary)    = 0x20  /* 'b', allow binary character */
};
#ifdef __cplusplus
	inline parser_allow() : sect(0xff), opt(0xff)
	{ }
#else
MPT_STRUCT(parser_allow)
{
# define MPT_PARSER_ALLOW_INIT { 0xff, 0xff }
#endif
	uint16_t sect,  /* section name format */
	         opt;   /* option name format */
};
typedef int (*MPT_TYPE(path_handler))(void *, const MPT_STRUCT(path) *, const MPT_STRUCT(value) *, int, int);

/* parser input metadata */
MPT_STRUCT(parser_input)
{
#ifdef __cplusplus
	parser_input();
#else
# define MPT_PARSER_INPUT_INIT { 0, 0, 1 }
#endif
	int (*getc)(void *); /* byte read operation */
	void *arg;           /* reader context */
	size_t line;         /* current line */
};
/* parser context */
#ifdef __cplusplus
MPT_STRUCT(parser_context)
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
	inline parser_context() : valid(0), prev(0), curr(0)
	{ }
#else
MPT_STRUCT(parser_context)
{
# define MPT_PARSER_INIT  { MPT_PARSER_INPUT_INIT, MPT_PARSER_ALLOW_INIT,  0,  0, 0 }
#endif
	MPT_STRUCT(parser_input) src;   /* character source */
	MPT_STRUCT(parser_allow) name;  /* section/option name format */
	
	uint16_t  valid;  /* valid size of post data */
	
	uint8_t   prev;   /* previous operation */
	uint8_t   curr;   /* current operation */
};
typedef int (*MPT_TYPE(input_parser))(void *, MPT_STRUCT(parser_context) *, MPT_STRUCT(path) *);

__MPT_EXTDECL_BEGIN

/* check option/section name according to limit flags */
extern int mpt_parse_ncheck(const char *, size_t , int);
/* set accept flags for name */
extern int mpt_parse_accept(MPT_STRUCT(parser_allow) *, const char *);


/* correct path */
extern int mpt_parse_post(MPT_STRUCT(path) *, int);


/* decode parameter and format type */
extern int mpt_parse_format(MPT_STRUCT(parser_format) *, const char *);
extern MPT_TYPE(input_parser) mpt_parse_next_fcn(int);


/* read character from source, save in 'post' path area */
extern int mpt_parse_getchar(MPT_STRUCT(parser_input) *, MPT_STRUCT(path) *);

/* continue to next visible character / end of line without saving */
extern int mpt_parse_nextvis(MPT_STRUCT(parser_input) *, const void *, size_t);
extern int mpt_parse_endline(MPT_STRUCT(parser_input) *);

/* get character from file */
extern int mpt_getchar_file(void *);
/* get character from stdio stream */
#if defined(_STDIO_H) || defined(_STDIO_H_)
extern int mpt_getchar_stdio(FILE *);
#endif
/* get character from IO vector */
extern int mpt_getchar_iovec(struct iovec *);


/* get element for prepending section names */
extern int mpt_parse_format_pre(const MPT_STRUCT(parser_format) *, MPT_STRUCT(parser_context) *, MPT_STRUCT(path) *);
/* get next element for encapsulated section names */
extern int mpt_parse_format_enc(const MPT_STRUCT(parser_format) *, MPT_STRUCT(parser_context) *, MPT_STRUCT(path) *);
/* get element for separating section names */
extern int mpt_parse_format_sep(const MPT_STRUCT(parser_format) *, MPT_STRUCT(parser_context) *, MPT_STRUCT(path) *);

/* get option element */
extern int mpt_parse_option(const MPT_STRUCT(parser_format) *, MPT_STRUCT(parser_context) *, MPT_STRUCT(path) *);
/* get data element */
extern int mpt_parse_data(const MPT_STRUCT(parser_format) *, MPT_STRUCT(parser_context) *, MPT_STRUCT(path) *);

/* create/modify current node element */
extern MPT_STRUCT(node) *mpt_node_append(MPT_STRUCT(node) *, const MPT_STRUCT(path) *, const MPT_STRUCT(value) *, int , int);
/* set node elements from file */
#ifdef _STDIO_H
extern int mpt_node_parse(MPT_STRUCT(node) *, FILE *, const char *, const char *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::default_instance()));
#endif
/* parse configuration tree */
extern int mpt_parse_config(MPT_TYPE(input_parser) , void *, MPT_STRUCT(parser_context) *, MPT_TYPE(path_handler), void *);
/* save config tree to node children */
extern int mpt_parse_node(MPT_STRUCT(node) *, MPT_STRUCT(parser_context) *, const char *);

#ifdef _DIRENT_H
/* load configuration folder */
extern int mpt_parse_folder(DIR *, MPT_TYPE(path_handler) , void *, MPT_INTERFACE(logger) *__MPT_DEFPAR(0));
#endif

__MPT_EXTDECL_END

#ifdef __cplusplus
class parser
{
public:
	parser();
	virtual ~parser();
	
	virtual bool reset();
	virtual bool set_format(const char *);
	virtual bool open(const char *);
	virtual int read(struct node &, logger * = logger::default_instance());
	
	inline size_t line() const
	{
		return _d.src.line;
	}
	inline const char *file() const
	{
		return _fn;
	}
protected:
	parser_context _d;
	struct {
		input_parser_t fcn;
		void *ctx;
	} _next;
	char *_fn;
};
inline bool parser::set_format(const char *)
{
	return false;
}
class config_parser : public parser
{
public:
	config_parser();
	
	bool reset() __MPT_OVERRIDE;
	bool set_format(const char *) __MPT_OVERRIDE;
protected:
	parser_format _fmt;
};
#endif

__MPT_NAMESPACE_END

#endif /* _MPT_PARSE_H */
