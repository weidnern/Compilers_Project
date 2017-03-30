/*
* definitions to support tree.c
*/

#ifndef TREE_H
#define TREE_H

#include "defs.h"
#include "types.h"
#include "symtab.h"

//enum of types that could be nodes in the tree
typedef enum{
	IDTN, PNTRTN, ARRAYTN, FUNCTN, REFTN
} TNODE_TYPE;

//tree node struct
typedef struct tnode {
	TNODE_TYPE type;
	union {
		ST_ID id; //if id type then store id
		//Nothing for pointer I think unless member list?
		unsigned int arr_size; //if array type then store the size of array
		PARAM_LIST plist; //if function, then store list of parameters
	} u;
	struct tnode *next; //points to next tnode in list
} TREE_NODE, *TNODE;


TNODE new_node(TNODE tn, TNODE_TYPE type);
TNODE push_node(TNODE tn, TNODE next);
TNODE pop_node(TNODE tn);
ST_ID get_id(TNODE tn);
TYPE get_type(TYPE t, TNODE tn);
BOOLEAN is_reference(TNODE tn);

#endif
