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
				ret = ty_build_func(ret, PROTOTYPE,node->u.plist); //Used PROTOTYPE instead of OLDSTYLE
				break;

			default:
				break;
		}
		node = node->next;
	}
	return ret;
}

BOOLEAN is_reference(TNODE tn)
{
	TNODE node = tn;
	while(node->next != NULL)
	{
		if(node->type == REFTN)
		{
			return TRUE;
		}
		node = node->next;
	}
	return FALSE;
}

//EXPRESSIONS

ENODE new_assop_node(EXPR_TYPE type, ASSOP_TYPE assop)
{
	ENODE new_node;
	new_node = (ENODE)malloc(sizeof(EXPRESSION_NODE));
	if(new_node == NULL)
		return NULL;
	new_node->type = type;
	new_node->u_expr.assop = assop;
	//new_node->next = en;
	//new_node->left = enl;
	//new_node->right = enr;
	return new_node;
}

ENODE new_binop_node(EXPR_TYPE type, BINOP_TYPE binop)
{
	ENODE new_node;
	new_node = (ENODE)malloc(sizeof(EXPRESSION_NODE));
	if(new_node == NULL)
		return NULL;
	new_node->type = type;
	new_node->u_expr.binop = binop;
	//new_node->next = en;
	return new_node;
}

ENODE new_unop_node(EXPR_TYPE type, UNOP_TYPE unop)
{
	ENODE new_node;
	new_node = (ENODE)malloc(sizeof(EXPRESSION_NODE));
	if(new_node == NULL)
		return NULL;
	new_node->type = type;
	new_node->u_expr.unop = unop;
	//new_node->next = en;
	return new_node;
}

ENODE new_comp_node(EXPR_TYPE type, COMP_TYPE comp)
{
	ENODE new_node;
	new_node = (ENODE)malloc(sizeof(EXPRESSION_NODE));
	if(new_node == NULL)
		return NULL;
	new_node->type = type;
	new_node->u_expr.comp = comp;
	//new_node->next = en;
	return new_node;
}

ENODE new_globalv_node(EXPR_TYPE type, GLOBALV_TYPE globalv)
{
	ENODE new_node;
	new_node = (ENODE)malloc(sizeof(EXPRESSION_NODE));
	if(new_node == NULL)
		return NULL;
	new_node->type = type;
	new_node->u_expr.globalv = globalv;
	//new_node->next = en;
	return new_node;
}

ENODE new_intconst_node(int con)
{
	ENODE new_node;
	new_node = (ENODE)malloc(sizeof(EXPRESSION_NODE));
	if(new_node == NULL)
		return NULL;
	new_node->type = INTCONST;
	new_node->u_expr.id_is_func = FALSE;
	return new_node;
}

ENODE new_fpconst_node(double con)
{
	ENODE new_node;
	new_node = (ENODE)malloc(sizeof(EXPRESSION_NODE));
	if(new_node == NULL)
		return NULL;
	new_node->type = FPCONST;
	new_node->u_expr.id_is_func = FALSE;
	return new_node;
}

ENODE new_id_node(ST_ID id)
{
	ENODE new_node;
	new_node = (ENODE)malloc(sizeof(EXPRESSION_NODE));
	if(new_node == NULL)
		return NULL;
	new_node->type = ID;
	int block;
	ST_DR stdr = st_lookup(id, &block);
	if(ty_query(stdr->u.decl.type) == TYFUNC)
		new_node->u_expr.id_is_func = TRUE;
	else
		new_node->u_expr.id_is_func = FALSE;
	return new_node;
}

ENODE new_func_node()
{
	ENODE new_node;
	new_node = (ENODE)malloc(sizeof(EXPRESSION_NODE));
	if(new_node == NULL)
		return NULL;
	new_node->type = PAREN;
	return new_node;
}
