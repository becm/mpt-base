/*!
 * read output parameter from configuration list.
 */

#include <stdio.h>
#include <string.h>

#include <sys/un.h>
#include <netinet/in.h>

#include "node.h"
#include "message.h"

#include "output.h"

/*!
 * \ingroup mptOutput
 * \brief set history parameters
 * 
 * get history parameters from configuration list.
 * 
 * \param out   output descriptor
 * \param conf  configuration list
 * 
 * \return combined result of history configuration
 */
extern int mpt_conf_history(MPT_INTERFACE(output) *out, const MPT_STRUCT(node) *conf)
{
	static const char data_def[] = "";
	MPT_INTERFACE(object) *obj;
	MPT_STRUCT(node) *tmp;
	const char *data;
	int e1, e2;
	
	if (!out || !(obj = out->_vptr->_mt.typecast((void *) out, MPT_ENUM(TypeObject)))) {
		return 0;
	}
	/* set history output format */
	if (!(tmp = conf ? mpt_node_next(conf, "outfmt") : 0)) {
		data = 0;
	} else if (!(data = mpt_node_data(tmp, 0))) {
		data = data_def;
	}
	e1 = mpt_object_set(obj, "histfmt", "s", data);
	
	/* set history output */
	if (!(tmp = conf ? mpt_node_next(conf, "outfile") : 0)) {
		data = 0;
	} else if (!(data = mpt_node_data(tmp, 0))) {
		data = data_def;
	}
	if ((e2 = mpt_object_set(obj, "histfile", "s", data)) < 0) {
		return (e1 < 0) ? e1 : (e1 ? 1 : 0);
	}
	if (e2) {
		return (e1 > 0) ? 3 : 2;
	}
	return (e1 > 0) ? 1 : 0;
}
