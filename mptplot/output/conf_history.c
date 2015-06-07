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
 * \param out	output descriptor
 * \param conf	configuration list
 * 
 * \return combined result of history configuration
 */
extern int mpt_conf_history(MPT_INTERFACE(output) *out, const MPT_STRUCT(node) *conf)
{
	static const char data_def[] = "";
	MPT_STRUCT(node) *tmp;
	MPT_STRUCT(property) pr;
	const char *data;
	int e1, e2;
	
	if (!out) return 0;
	
	/* set history output format */
	if (!(tmp = conf ? mpt_node_next(conf, "outfmt") : 0)) {
		data = 0;
	} else if (!(data = mpt_node_data(tmp, 0))) {
		data = data_def;
	}
	
	pr.name = "histfmt";
	pr.desc = 0;
	pr.val.fmt = 0;
	pr.val.ptr = data;
	
	e1 = mpt_meta_pset((void *) out, &pr, 0);
	
	/* set history output */
	if (!(tmp = conf ? mpt_node_next(conf, "outfile") : 0)) {
		data = 0;
	} else if (!(data = mpt_node_data(tmp, 0))) {
		data = data_def;
	}
	
	pr.name = "histfile";
	pr.desc = 0;
	pr.val.fmt = 0;
	pr.val.ptr = data;
	
	e2 = mpt_meta_pset((void *) out, &pr, 0);
	
	if (e1 < 0) return e2;
	if (e2 < 0) return e1;
	
	return e1 | e2;
}
