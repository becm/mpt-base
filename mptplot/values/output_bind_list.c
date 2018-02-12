/*!
 * set output bindings from node list.
 */

#include <ctype.h>
#include <string.h>
#include <limits.h>

#include <sys/uio.h>

#include "node.h"
#include "meta.h"
#include "message.h"

#include "output.h"

/*!
 * \ingroup mptOutput
 * \brief assign data bindings
 * 
 * Send data bindings in node list to output.
 * 
 * \param out  output descriptor
 * \param conf configuration list containing data bindings
 * 
 * \return number of bindings
 */
extern int mpt_output_bind_list(MPT_INTERFACE(output) *out, const MPT_STRUCT(node) *conf)
{
	int nbind = 0;
	
	if (!conf) {
		return MPT_ERROR(BadArgument);
	}
	do {
		MPT_INTERFACE(metatype) *meta;
		const char *src, *dest;
		MPT_STRUCT(msgtype) mt;
		ssize_t len;
		
		if (!(dest = mpt_node_ident(conf))) {
			continue;
		}
		if (!(meta = conf->_meta) || !(src = mpt_meta_data(meta, 0))) {
			continue;
		}
		mt.cmd = MPT_MESGTYPE(Graphic);
		mt.arg = MPT_MESGGRF(BindingAdd) | MPT_MESGGRF(BindingParse);
		
		if ((len = out->_vptr->await(out, 0, 0)) < 0) {
			return nbind ? nbind : -1;
		}
		if ((len = out->_vptr->push(out, sizeof(mt), &mt)) >= 0) {
			if ((len = out->_vptr->push(out, strlen(src)+1, src)) < 0
			    || (len = out->_vptr->push(out, strlen(dest), dest)) < 0
			    || (len = out->_vptr->push(out, 0, 0)) < 0) {
				out->_vptr->push(out, 1, 0);
			}
		}
		if (len < 0) {
			return nbind ? nbind : -1;
		}
	} while ((conf = conf->next));
	
	return nbind;
}
