/*!
 * MPT client library
 *  creation and operations on solver extension
 */

#ifndef _MPT_CLIENT_H
#define _MPT_CLIENT_H  @INTERFACE_VERSION@

#include "meta.h"
#include "output.h"

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(event);
MPT_STRUCT(message);
MPT_INTERFACE(iterator);

#ifdef __cplusplus
MPT_INTERFACE(client) : public metatype
{
public:
	/* metatype interface */
	int convert(type_t , void *) __MPT_OVERRIDE;
	
	/* client extensions */
	virtual int dispatch(event * = 0);
	virtual int process(uintptr_t , iterator * = 0) = 0;
	
	static class config &config();
	
	static const struct named_traits *pointer_traits();
	
	enum { LogStatus = logger::Debug2 };
protected:
	inline ~client() { }
};
template <> int type_properties<client *>::id(bool);
template <> const MPT_STRUCT(type_traits) *type_properties<client *>::traits();

#else
# define MPT_CLIENT_LOG_STATUS MPT_LOG(Debug2)
MPT_INTERFACE(client);
MPT_INTERFACE_VPTR(client)
{
	MPT_INTERFACE_VPTR(metatype) meta;
	int (*dispatch)(MPT_INTERFACE(client) *, MPT_STRUCT(event) *);
	int (*process)(MPT_INTERFACE(client) *, uintptr_t , MPT_INTERFACE(iterator) *);
}; MPT_INTERFACE(client) {
	const MPT_INTERFACE_VPTR(client) *_vptr;
};
#endif

__MPT_EXTDECL_BEGIN

/* get client id */
extern const MPT_STRUCT(named_traits) *mpt_client_type_traits();

/* process command message */
extern int mpt_client_command(MPT_INTERFACE(client) *, const MPT_STRUCT(message) *, int);

__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_CLIENT_H */
