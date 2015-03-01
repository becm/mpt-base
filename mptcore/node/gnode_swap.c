/*!
 * replace nodes
 */

#include <errno.h>

#include "node.h"

/*!
 * \ingroup mptNode
 * \brief swap node children
 * 
 * Swap children between nodes.
 */
extern void mpt_gnode_swap(MPT_STRUCT(node) *pri, MPT_STRUCT(node) *sec)
{
	/* relocate node children */
	MPT_STRUCT(node) *pc, *sc;
	
	pc = pri->children;
	sc = sec->children;
	sec->children = pc;
	pri->children = sc;
	
	while (pc) {
		pc->parent = sec;
		pc = pc->next;
	}
	while (sc) {
		sc->parent = pri;
		sc = sc->next;
	}
}

/*!
 * \ingroup mptNode
 * \brief switch nodes
 * 
 * Switch node associations between nodes except children.
 * Eqivalent to switch subtrees.
 */
extern void mpt_gnode_switch(MPT_STRUCT(node) *pri, MPT_STRUCT(node) *sec)
{
	MPT_STRUCT(node) *parent, *next, *prev, *tmp;
	
	/* save node pointers */
	parent	= pri->parent;
	next	= pri->next;
	prev	= pri->prev;
	
	/* reassign primary */
	if ((pri->next = tmp = sec->next)) {
		tmp->prev = pri;
	}
	else if ((pri->parent = tmp = sec->parent)
	         && tmp->children == sec) {
		tmp->children = pri;
	}
	if ((pri->prev = tmp = sec->prev)) {
		tmp->next = pri;
	}
	/* reassign secondary */
	if ((sec->next = next)) {
		next->prev = sec;
	}
	else if ((sec->parent = parent)
	         && parent->children == pri) {
		parent->children = sec;
	}
	if ((sec->prev = prev)) {
		prev->next = sec;
	}
}

