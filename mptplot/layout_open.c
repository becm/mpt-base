
#include <string.h>
#include <strings.h>

#include "output.h"
#include "message.h"

#include "values.h"

static int setLayout(void *ln, const MPT_STRUCT(message) *cmsg)
{
	MPT_STRUCT(message) msg;
	struct {
		MPT_STRUCT(msgtype) mt;
		MPT_STRUCT(laydest) d;
	} info;
	size_t mlen = sizeof(info.mt) + sizeof(info.d.lay);
	
	if (!cmsg) {
		return 0;
	}
	msg = *cmsg;
	
	if (mpt_message_read(&msg, sizeof(info), &info) < mlen) {
		return -1;
	}
	if ((info.mt.cmd == MPT_MESGTYPE(Graphic))
	    && (info.mt.arg == (int8_t) (MPT_MESGGRF(LayoutOpen)))) {
		*(int *) ln = info.d.lay;
		return 0;
	}
	else if (!info.mt.cmd && !info.mt.arg) {
		*(int *) ln = 0;
		return 1;
	}
	return 2;
}

/*!
 * \ingroup mptOutput
 * \brief open layout
 * 
 * Send command to read layout.
 * 
 * \param out   output descriptor
 * \param desc  layout description/filename
 * \param gen   gererator mode
 */
extern int mpt_layout_open(MPT_INTERFACE(output) *out, const char *desc, const char *gen)
{
	static int lid;
	MPT_STRUCT(msgtype) mt;
	ssize_t len;
	int err;
	
	mt.cmd = MPT_MESGTYPE(Graphic);
	mt.arg = MPT_MESGGRF(LayoutOpen);
	
	lid = -1;
	
	/* specify generator mode */
	if (gen) {
		if (!strcasecmp("file", gen)) {
			mt.arg = MPT_MESGGRF(LayoutOpen);
		}
		else if (!strcasecmp("lay", gen) || !strcasecmp("layout", gen)) {
			mt.arg = MPT_MESGGRF(LayoutParse);
		}
		else if (!strcasecmp("gen", gen) || !strcasecmp("generate", gen) || !strcasecmp("create", gen)) {
			mt.arg = MPT_MESGGRF(LayoutCreate);
		}
		else {
			return -1;
		}
	}
	/* register layout id setter */
	if ((err = out->_vptr->await(out, setLayout, &lid)) < 0) {
		return err;
	}
	/* begin open operation */
	if ((len = out->_vptr->push(out, sizeof(mt), &mt)) < 0) {
		return len;
	}
	/* add layout description/filename */
	if (desc) {
		len = out->_vptr->push(out, strlen(desc)+1, desc);
	}
	/* finish message */
	if (len < 0 || (len = out->_vptr->push(out, 0, 0)) < 0) {
		out->_vptr->push(out, 1, 0);
		return len;
	}
	/* wait for answers to queries */
	out->_vptr->sync(out, -1);
	
	/* answer not arrived within call */
	if (lid < 0) {
		return 0;
	}
	return lid;
}
