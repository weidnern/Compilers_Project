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
	free(temp)
	return tn;
}
