/*
* tree.c
*
* This class builds the syntax tree of a declaration.
*/

#include <stdlib.h>
#include "tree.h"
#include "types.h"
#include "symtab.h"
#include "defs.h"
#include "message.h"



TNODE new_node(TNODE tn, TNODE_TYPE type)
{
	TNODE new_node;
	new_node = (TNODE)malloc(sizeof(TREE_NODE));
	if(new_node == NULL)
		return NULL;
	new_node->type = type;
	new_node->next = tn;
	return new_node;
}

//Make sure I ordered this right
TNODE push_node(TNODE tn, TNODE next)
{
	next->next = tn;
	tn = next;
	return tn;
}

//Make sure I ordered this right too
TNODE pop_node(TNODE tn)
{
	TNODE temp = tn;
	tn = tn->next;
	free(temp);
	return tn;
}

ST_ID get_id(TNODE tn)
{
	TNODE node = tn;
	while(node->next != NULL)
	{
		node = node->next;
	}
	return node->u.id;
}

TYPE get_type(TYPE t, TNODE tn)
{
	TYPE ret = t;
	TNODE node = tn;

	while(node->next != NULL)
	{
		switch(node->type)
		{
			case PNTRTN:
				ret = ty_build_ptr(ret, NO_QUAL);
				break;

			case ARRAYTN:
				ret = ty_build_array(ret, DIM_PRESENT, node->u.arr_size);
				break;

			case FUNCTN:
				ret = ty_build_func(ret, OLDSTYLE,node->u.plist);
				break;

			default:
				break;
		}
	}

	return ret;
}