/*!
 * MPT plotting library
 *  output operations
 */

#ifndef _MPT_OUTPUT_H
#define _MPT_OUTPUT_H  201401


#include "core.h"

#ifdef __cplusplus
# include "message.h"
#endif

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(node);
MPT_STRUCT(array);
MPT_STRUCT(notify);

MPT_INTERFACE(output);

/* source data type */
MPT_STRUCT(msgbind)
{
#ifdef __cplusplus
	inline msgbind(int d, int m = OutputStateInit | OutputStateStep) : dim(d), type(m)
	{ }
#else
# define MPT_MSGBIND_INIT { 0, (MPT_ENUM(OutputStateInit) | MPT_ENUM(OutputStateStep)) }
#endif
	uint8_t dim,   /* source dimension */
	        type;  /* type of data */
};
/* layout destination */
MPT_STRUCT(laydest)
{
#ifdef __cplusplus
	inline laydest(uint8_t l = 0, uint8_t g = 0, uint8_t w = 0, uint8_t d = 0) :
		lay(l), grf(g), wld(w), dim(d)
	{ }
	inline bool operator==(const laydest &ld)
	{ return lay == ld.lay && grf == ld.grf && wld == ld.wld; }
	inline bool same(const laydest &ld)
	{ return *this == ld && dim == ld.dim; }
#else
# define MPT_LAYDEST_INIT { 0, 0, 0, 0 }
#endif
	uint8_t lay,  /* target layout */
	        grf,  /* target graph */
	        wld,  /* target world */
	        dim;  /* target dimension */
};

MPT_STRUCT(histinfo)
{
# ifdef __cplusplus
public:
	inline histinfo() : fmt(0), pos(0), part(0), line(0), type(0)
	{ }
	inline ~histinfo()
	{ free(fmt); }
	
	bool setFormat(const char *fmt);
	bool setup(size_t , const msgbind *);
protected:
# else
#  define MPT_HISTINFO_INIT  { 0,  0, 0, 0,  0, 0 }
# endif
	int16_t  *fmt;   /* float output format */
	uint16_t  pos;   /* position in line */
	uint16_t  part;  /* part of line to display */
	uint16_t  line;  /* line lenth */
	char      type;  /* type information */
	uint8_t   size;  /* element size */
};

MPT_STRUCT(outdata)
#ifdef _MPT_ARRAY_H
{
# ifdef __cplusplus
	outdata();
	
	ssize_t push(size_t, const void *);
protected:
# else
#  define MPT_OUTDATA_INIT { MPT_SOCKET_INIT,  0,0,0,0,  MPT_ARRAY_INIT, { 0, MPT_CODESTATE_INIT }, 0 }
# endif
	MPT_STRUCT(socket)     sock;
	uint8_t               _sflg;    /* socket flags */
	uint8_t                state;   /* output state */
	uint8_t                level;   /* output level */
	uint8_t                curr;    /* type of active message */
	MPT_STRUCT(array)     _buf;
	struct {
		MPT_TYPE(DataEncoder) fcn;
		MPT_STRUCT(codestate) info;
	} _enc;
# ifdef _STDIO_H
	FILE                  *hist;
# else
	void                  *hist;
# endif
}
#endif
;
/* binding to layout mapping */
MPT_STRUCT(mapping)
{
#ifdef __cplusplus
	inline mapping(const msgbind &m = msgbind(0), const laydest &d = laydest(), int c = 0) :
		src(m), client(c), dest(d)
	{ }
	inline bool valid()
	{ return src.type != 0; }
#else
# define MPT_MAPPING_INIT { MPT_MSGBIND_INIT, 0, MPT_LAYDEST_INIT }
#endif
	MPT_STRUCT(msgbind) src;
	uint16_t            client;
	MPT_STRUCT(laydest) dest;
};

/* world position */
MPT_STRUCT(msgworld)
{
#ifdef __cplusplus
	inline msgworld() : cycle(0), offset(0) { }
#endif
	int32_t cycle,   /* target cycle (<0: offset from current) */
	        offset;  /* data offset (<0: append) */
};

MPT_STRUCT(msgval)
{
#ifdef __cplusplus
	msgval(uint count, const double *from, int ld = 1);
#else
# define MPT_MSGVAL_INIT { 0, 0, 0, 0 }
#endif
	const void    *base;  /* data base address */
	void         (*copy)(int , const void *, int , void *, int);
	unsigned int   elem;  /* remaining elements */
	int            ld;    /* leading dimension */
};

MPT_STRUCT(strdest)
{
	uint8_t change,  /* positions which were changed */
	        val[7];  /* values before/after reading */
};

__MPT_EXTDECL_BEGIN

/* decode string to MPT destination */
extern int mpt_string_dest(MPT_STRUCT(strdest) *, int , const char *);

/* configure graphic output and bindings */
extern int mpt_conf_graphic(MPT_INTERFACE(output) *, const MPT_STRUCT(node) *);
/* configure history output and format */
extern int mpt_conf_history(MPT_INTERFACE(output) *, const MPT_STRUCT(node) *);

/* send layout open command */
extern int mpt_layout_open(MPT_INTERFACE(output) *, const char *, const char *);

/* parse graphic binding */
extern int mpt_outbind_set(MPT_STRUCT(msgbind) *, const char *);
/* set output bindings */
extern int mpt_outbind_list(MPT_INTERFACE(output) *, const MPT_STRUCT(node) *);
extern int mpt_outbind_string(MPT_INTERFACE(output) *, const char *);

/* data output formats */
extern int mpt_output_data(MPT_INTERFACE(output) *, int, int , int , const double *, int);
extern int mpt_output_history(MPT_INTERFACE(output) *, int, const double *, int, const double *, int);
extern int mpt_output_values(MPT_INTERFACE(output) *, const MPT_STRUCT(msgval) *, size_t);
extern int mpt_output_plot(MPT_INTERFACE(output) *, const MPT_STRUCT(laydest) *, int, const double *, int);

/* history operations */
extern int mpt_history_setfmt(MPT_STRUCT(histinfo) *, MPT_INTERFACE(source) *);
extern int mpt_history_set(MPT_STRUCT(histinfo) *, const MPT_STRUCT(msgbind) *);
#ifdef _STDIO_H
extern ssize_t mpt_history_print(FILE *, MPT_STRUCT(histinfo) *, size_t , const void *);

/* printing values */
extern int mpt_fprint_int(FILE *, const int8_t  *, MPT_INTERFACE(source) *);
extern int mpt_fprint_float(FILE *, const int16_t *, MPT_INTERFACE(source) *);
#endif

/* clear outdata */
extern void mpt_outdata_fini(MPT_STRUCT(outdata) *);
/* modify outdata */
extern int mpt_outdata_property(MPT_STRUCT(outdata) *, MPT_STRUCT(property) *, MPT_INTERFACE(source) *);
/* push to outdata */
extern ssize_t mpt_outdata_push(MPT_STRUCT(outdata) *, size_t , const void *);
/* outdata print setup and processing */
extern int mpt_outdata_print(MPT_STRUCT(outdata) *, size_t , const void *);


/* data mapping operations */
extern int mpt_mapping_add(MPT_STRUCT(array) *, const MPT_STRUCT(mapping) *);
extern int mpt_mapping_del(const MPT_STRUCT(array) *, const MPT_STRUCT(msgbind) *, const MPT_STRUCT(laydest) * __MPT_DEFPAR(0), int __MPT_DEFPAR(0));
extern const MPT_STRUCT(mapping) *mpt_mapping_get(const MPT_STRUCT(array) *, const MPT_STRUCT(msgbind) *, int __MPT_DEFPAR(0));


/* create output instance */
extern MPT_INTERFACE(output) *mpt_output_new(MPT_STRUCT(notify) * __MPT_DEFPAR(0));

__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_OUTPUT_H */
