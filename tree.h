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

//enum if exrepssion types
typedef enum{
	ASSOP, BINOP, UNOP, INTCONST, FPCONST, COMP, GLOBALV, PAREN, FCALL
} EXPR_TYPE;

//enums of specific expression
typedef enum {
	EQUALS
} ASSOP_TYPE;

typedef enum {
	PLUS, MINUS, TIMES, DIVIDE
} BINOP_TYPE;

typedef enum {
	UMINUS
} UNOP_TYPE;

typedef enum {
	DOUBLE_EQUALS, LESS_THAN, GREATER_THAN, NOT_EQUAL, LESS_THAN_EQUAL, GREATER_THAN_EQUAL
} COMP_TYPE;

typedef enum {
	GINT, GCHAR, GFLOAT, GDOUBLE
} GLOBALV_TYPE;

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

typedef struct enode {
	EXPR_TYPE type;
	union {
		ASSOP_TYPE assop;
		BINOP_TYPE binop;
		UNOP_TYPE unop;
		COMP_TYPE comp;
		GLOBALV_TYPE globalv;
	} u_expr;
	struct enode *left;
	struct enode *right;
	struct enode *next;
} EXPRESSION_NODE, *ENODE;


TNODE new_node(TNODE tn, TNODE_TYPE type);
TNODE push_node(TNODE tn, TNODE next);
TNODE pop_node(TNODE tn);
ST_ID get_id(TNODE tn);
TYPE get_type(TYPE t, TNODE tn);
BOOLEAN is_reference(TNODE tn);

//EXPRESSIONS
ENODE new_assop_node(EXPR_TYPE type, ASSOP_TYPE assop);
ENODE new_binop_node(ENODE en, EXPR_TYPE type, BINOP_TYPE binop);
ENODE new_unop_node(ENODE en, EXPR_TYPE type, UNOP_TYPE unop);
ENODE new_comp_node(ENODE en, EXPR_TYPE type, COMP_TYPE comp);
ENODE new_globalv_node(ENODE en, EXPR_TYPE type, GLOBALV_TYPE globalv);

#endif
