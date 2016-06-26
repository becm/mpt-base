
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "queue.h"
#include "stream.h"

#include "array.h"
#include "message.h"

#include "meta.h"
#include "convert.h"

#include "output.h"

static int outdataConnection(MPT_STRUCT(outdata) *out, MPT_INTERFACE(metatype) *src)
{
	char *where;
	int len;
	
	/* use default connection */
	if (!src) {
		where = "Ip:[::]:16565";
		len = 0;
	}
	if (out->state & MPT_ENUM(OutputActive)) {
		return MPT_ERROR(BadOperation);
	}
	if (!src || (len = src->_vptr->conv(src, 's', &where)) >= 0) {
		int ret;
		if ((ret = mpt_outdata_open(out, where, 0)) < 0) {
			return ret;
		}
		return 1;
	}
	return MPT_ERROR(BadType);
}

static int outputEncoding(MPT_STRUCT(outdata) *out, MPT_INTERFACE(metatype) *src)
{
	MPT_TYPE(DataEncoder) enc;
	MPT_TYPE(DataDecoder) dec;
	MPT_STRUCT(stream) *srm;
	char *where;
	int32_t val;
	int res;
	uint8_t rtype;
	
	if (!src) {
		rtype = out->_coding;
	}
	else if ((res = src->_vptr->conv(src, 's', &where)) >= 0) {
		val = mpt_encoding_value(where, -1);
		if (val < 0 || val > UINT8_MAX) {
			return -2;
		}
		rtype = val;
	}
	else if ((res = src->_vptr->conv(src, 'y', &rtype)) < 0) {
		res = src->_vptr->conv(src, 'i', &val);
		if (res < 0 || val < 0 || val > UINT8_MAX) {
			return -3;
		}
		rtype = val;
	}
	if (!rtype) {
		/* active stream needs encoding */
		if (!MPT_socket_active(&out->sock) && out->_buf) {
			return MPT_ERROR(BadValue);
		}
		enc = 0;
		dec = 0;
	}
	else if (!(enc = mpt_message_encoder(rtype))
	    || !(dec = mpt_message_decoder(rtype))) {
		return -3;
	}
	if (MPT_socket_active(&out->sock) || !(srm = out->_buf)) {
		out->_coding = rtype;
		return res;
	}
	if (srm->_wd._enc) {
		srm->_wd._enc(&srm->_wd._state, 0, 0);
		srm->_wd._enc = enc;
	}
	if (srm->_rd._dec) {
		srm->_rd._dec(&srm->_rd._state, 0, 0);
		srm->_rd._dec = dec;
	}
	out->_coding = rtype;
	
	return res;
}

static int outdataColor(uint8_t *flags, MPT_INTERFACE(metatype) *src)
{
	char *where;
	int len;
	int32_t val = 0;
	
	if (!src) {
		*flags &= ~MPT_ENUM(OutputPrintColor);
		return 0;
	}
	if ((len = src->_vptr->conv(src, 's', &where)) >= 0) {
		if (!where || *where || !strcasecmp(where, "false")) {
			val = 1;
		} else {
			val = 0;
		}
	}
	else if ((len = src->_vptr->conv(src, 'i', &val)) < 0) {
		return MPT_ERROR(BadType);
	}
	if (val > 0) {
		*flags |= MPT_ENUM(OutputPrintColor);
	} else {
		*flags &= ~MPT_ENUM(OutputPrintColor);
	}
	return len;
}

/*!
 * \ingroup mptOutput
 * \brief query outdata properties
 * 
 * Set/Get properties of outdata.
 * 
 * \param sock target output descriptor
 * \param name property to query
 * \param src  data source to change property
 */
extern int mpt_outdata_set(MPT_STRUCT(outdata) *od, const char *name, MPT_INTERFACE(metatype) *src)
{
	if (!name) {
		return src ? outdataConnection(od, src) : MPT_ENUM(TypeSocket);
	}
	/* cast operation */
	if (!*name) {
		return outdataConnection(od, src);
	}
	if (!strcasecmp(name, "encoding")) {
		return outputEncoding(od, src);
	}
	if (!strcasecmp(name, "color")) {
		return outdataColor(&od->state, src);
	}
	return MPT_ERROR(BadArgument);
}

/*!
 * \ingroup mptOutput
 * \brief query outdata properties
 * 
 * Get properties of outdata.
 * 
 * \param od   output data descriptor
 * \param prop property data
 */
extern int mpt_outdata_get(const MPT_STRUCT(outdata) *od, MPT_STRUCT(property) *pr)
{
	const char *name;
	intptr_t pos = -1, id;
	
	if (!pr) {
		return MPT_ENUM(TypeSocket);
	}
	/* find by position */
	if (!(name = pr->name)) {
		pos = (intptr_t) pr->desc;
	}
	else if (!*name) {
		static const char fmt[2] = { MPT_ENUM(TypeSocket), 0 };
		
		pr->name = "outdata";
		pr->desc = "output data context";
		pr->val.fmt = fmt;
		pr->val.ptr = &od->sock;
		
		return MPT_socket_active(&od->sock) ? 1 : 0;
	}
	id = 0;
	if (name ? !strcmp(name, "color") : pos == id++) {
		pr->name = "color";
		pr->desc = MPT_tr("colorized message output");
		pr->val.fmt = 0;
		if (od->state & MPT_ENUM(OutputPrintColor)) {
			pr->val.ptr = "true";
			return MPT_ENUM(OutputPrintColor);
		} else {
			pr->val.ptr = "false";
			return 0;
		}
	}
	return MPT_ERROR(BadArgument);
}

