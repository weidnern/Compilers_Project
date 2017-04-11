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
	ASSOP, BINOP, UNOP, ID, INTCONST, FPCONST, COMP, GLOBALV, PAREN, FCALL
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
	EXPR_TYPE expr_type;
	TYPE type;
	union {
		long intval;
		double doubleval;
		ST_ID id;
		struct {
			UNOP_TYPE op;
			struct enode * arg;
		} unop;
		struct {
			BINOP_TYPE op;
			struct enode *left, *right;
		} binop;
		struct {
			struct enode * fname;
			//EXPR_LIST actual_args;
		} fcall;
		struct {
			ASSOP_TYPE op;
			struct enode *left, *right;
		} assop;
		struct {
			COMP_TYPE op;
			struct enode *left, *right;
		} comp;
	} u_expr;
} EXPRESSION_NODE, *ENODE;


TNODE new_node(TNODE tn, TNODE_TYPE type);
TNODE push_node(TNODE tn, TNODE next);
TNODE pop_node(TNODE tn);
ST_ID get_id(TNODE tn);
TYPE get_type(TYPE t, TNODE tn);
BOOLEAN is_reference(TNODE tn);

//EXPRESSIONS
ENODE new_assop_node(ASSOP_TYPE assop, ENODE left, ENODE right);
ENODE new_binop_node(BINOP_TYPE binop, ENODE left, ENODE right);
ENODE new_unop_node(UNOP_TYPE unop, ENODE arg);
ENODE new_comp_node(COMP_TYPE comp, ENODE left, ENODE right);
ENODE new_globalv_node(GLOBALV_TYPE globalv);
ENODE new_intconst_node(int con);
ENODE new_fpconst_node (double con);
ENODE new_id_node(ST_ID id);
ENODE new_func_node();

#endif
