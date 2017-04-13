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

ENODE new_assop_node(ASSOP_TYPE assop, ENODE left, ENODE right)
{
	ENODE new_node;
	new_node = (ENODE)malloc(sizeof(EXPRESSION_NODE));
	if(new_node == NULL)
		return NULL;
	new_node->expr_type = ASSOP;
	new_node->type = right->type;
	new_node->u_expr.assop.op = assop;
	new_node->u_expr.assop.left = left;
	new_node->u_expr.assop.right = right;
	return new_node;
}

ENODE new_binop_node(BINOP_TYPE binop, ENODE left, ENODE right)
{
	ENODE new_node;
	new_node = (ENODE)malloc(sizeof(EXPRESSION_NODE));
	if(new_node == NULL)
		return NULL;

	//error("left child type: %d", left->expr_type);
	//error("right child type: %d", right->expr_type);
	if(left->expr_type == FPCONST && right->expr_type == INTCONST)
	{
		switch(binop)
		{
			case PLUS:
				new_node = new_fpconst_node(left->u_expr.doubleval + (double)right->u_expr.intval);
				break;

			case MINUS:
				new_node = new_fpconst_node(left->u_expr.doubleval - (double)right->u_expr.intval);
				break;

			case TIMES:
				new_node = new_fpconst_node(left->u_expr.doubleval * (double)right->u_expr.intval);
				break;

			case DIVIDE:
				new_node = new_fpconst_node(left->u_expr.doubleval / (double)right->u_expr.intval);
				break;

			default:
				break;
		}
	}
	else if(left->expr_type == INTCONST && right->expr_type == FPCONST)
	{
		switch(binop)
		{
			case PLUS:
				new_node = new_fpconst_node((double)left->u_expr.intval + right->u_expr.doubleval);
				break;

			case MINUS:
				new_node = new_fpconst_node((double)left->u_expr.intval - right->u_expr.doubleval);
				break;

			case TIMES:
				new_node = new_fpconst_node((double)left->u_expr.intval * right->u_expr.doubleval);
				break;

			case DIVIDE:
				new_node = new_fpconst_node((double)left->u_expr.intval / right->u_expr.doubleval);
				break;

			default:
				break;
		}
	}
	else if(left->expr_type == INTCONST && right->expr_type == INTCONST)
	{
		switch(binop)
		{
			case PLUS:
				new_node = new_intconst_node(left->u_expr.intval + right->u_expr.intval);
				break;

			case MINUS:
				new_node = new_intconst_node(left->u_expr.intval - right->u_expr.intval);
				break;

			case TIMES:
				new_node = new_intconst_node(left->u_expr.intval * right->u_expr.intval);
				break;

			case DIVIDE:
				new_node = new_intconst_node(left->u_expr.intval / right->u_expr.intval);
				break;

			default:
				break;
		}
	}
	else if(left->expr_type == FPCONST && right->expr_type == FPCONST)
	{
		switch(binop)
		{
			case PLUS:
				new_node = new_fpconst_node(left->u_expr.doubleval + right->u_expr.doubleval);
				break;

			case MINUS:
				new_node = new_fpconst_node(left->u_expr.doubleval - right->u_expr.doubleval);
				break;

			case TIMES:
				new_node = new_fpconst_node(left->u_expr.doubleval * right->u_expr.doubleval);
				break;

			case DIVIDE:
				new_node = new_fpconst_node(left->u_expr.doubleval / right->u_expr.doubleval);
				break;

			default:
				break;
		}
	}
	else
	{
		//Only set these if we aren't constant folding
		new_node->expr_type = BINOP;
		new_node->type = right->type;
		new_node->u_expr.binop.op = binop;
		new_node->u_expr.binop.left = left;
		new_node->u_expr.binop.right = right;
	}
	return new_node;
}

ENODE new_unop_node(UNOP_TYPE unop, ENODE arg)
{
	ENODE new_node;
	new_node = (ENODE)malloc(sizeof(EXPRESSION_NODE));
	if(new_node == NULL)
		return NULL;
	new_node->expr_type = UNOP;
	new_node->type = arg->type;
	new_node->u_expr.unop.op = unop;
	new_node->u_expr.unop.arg = arg;
	return new_node;
}

ENODE new_comp_node(COMP_TYPE comp, ENODE left, ENODE right)
{
	ENODE new_node;
	new_node = (ENODE)malloc(sizeof(EXPRESSION_NODE));
	if(new_node == NULL)
		return NULL;
	new_node->expr_type = COMP;
	new_node->type = right->type;
	new_node->u_expr.comp.op = comp;
	new_node->u_expr.comp.left = left;
	new_node->u_expr.comp.right = right;
	return new_node;
}

ENODE new_globalv_node(GLOBALV_TYPE globalv)
{
	ENODE new_node;
	new_node = (ENODE)malloc(sizeof(EXPRESSION_NODE));
	if(new_node == NULL)
		return NULL;
	new_node->expr_type = GLOBALV;
	//new_node->u_expr.globalv = globalv;
	//new_node->next = en;
	return new_node;
}

ENODE new_intconst_node(int con)
{
	ENODE new_node;
	new_node = (ENODE)malloc(sizeof(EXPRESSION_NODE));
	if(new_node == NULL)
		return NULL;
	new_node->expr_type = INTCONST;
	new_node->type = ty_build_basic(TYSIGNEDINT);
	new_node->u_expr.intval = con;
	return new_node;
}

ENODE new_fpconst_node(double con)
{
	ENODE new_node;
	new_node = (ENODE)malloc(sizeof(EXPRESSION_NODE));
	if(new_node == NULL)
		return NULL;
	new_node->expr_type = FPCONST;
	new_node->type = ty_build_basic(TYDOUBLE);
	new_node->u_expr.doubleval = con;
	return new_node;
}

ENODE new_id_node(ST_ID id)
{
	ENODE new_node;
	new_node = (ENODE) malloc(sizeof(EXPRESSION_NODE));
	if (new_node == NULL)
		fatal("No more dynamic memory!");
	int block;
	ST_DR stdr = st_lookup(id, &block);
	if(stdr != NULL)
	{
		new_node->expr_type = ID;
		new_node->type = stdr->u.decl.type;
		new_node->u_expr.id = id;
	}
	else
	{
		error("%s is undefined", st_get_id_str(id));
		//free(new_node);
		new_node->type = NULL;
	}
	return new_node;
}

ENODE new_func_node(ENODE name, EXPR_LIST args)
{
	ST_ID id;
	ENODE new_node;
	new_node = (ENODE) malloc(sizeof(EXPRESSION_NODE));
	if (new_node == NULL)
		fatal("No more dynamic memory!");
	if(name->expr_type == ID)
	{
		id = name->u_expr.id;
		int block;
		ST_DR stdr = st_lookup(id, &block);
		if(stdr != NULL)
		{
			new_node->expr_type = FCALL;
			new_node->type = stdr->u.decl.type;
			new_node->u_expr.fcall.fname = name;
			new_node->u_expr.fcall.actual_args = args;
			return new_node;
		}
		else
		{
			error("ID is not in symbol table");
			free(new_node);
			free(stdr);
		}
	}
	else
	{
		error("Does not have an ID");
		free(new_node);
	}	
}

EXPR_LIST make_expr_list_node(ENODE expr, EXPR_LIST list)
{
	EXPR_LIST node = (EXPR_LIST) malloc(sizeof(EXPR_LIST_NODE));
	node->expr = expr;
	node->prev = list;
	if(list != NULL) list->next = node;
	else node->next = NULL;
	return node;	
}
