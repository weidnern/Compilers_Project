/*
* encode.c
*
* This class contains the code generation routines.
*/

#include "encode.h"
#include "types.h"
#include "message.h"
#include "defs.h"
#include "string.h"
#include "symtab.h"
#include "tree.h"
#include "backend-x86.h"

unsigned int get_size(TYPE t)
{
	unsigned int ret = 0;
	
	TYPETAG tag = ty_query(t);
	if(tag == TYARRAY)
	{
		unsigned int dim;
    	DIMFLAG dimflag;
		TYPE ta = ty_query_array(t, &dimflag, &dim);
		TYPETAG ta_tag = ty_query(ta);
		unsigned int tmp = dim;
		if(ta_tag == TYFUNC) {
			error("Cannot have array of functions.");
			return -1; //could cause errors later
		}
		while(ta_tag == TYARRAY)
		{	
			unsigned int ta_dim;
    		DIMFLAG ta_dimflag;
			ta = ty_query_array(ta, &ta_dimflag, &ta_dim);
			ta_tag = ty_query(ta);
			tmp = tmp * ta_dim;
		}
		unsigned int ta_size = get_size_basic(ta_tag);
		if(tmp*ta_size==0) error("Illegal array dimension.");
		else {
			ret = tmp * ta_size;
		}
	}
	else if(tag == TYPTR) {
		ret = 4;
	}
	else if(tag == TYFUNC) {
		PARAMSTYLE paramstyle;
		PARAM_LIST params;
		TYPE ta = ty_query_func(t, &paramstyle, &params);
		TYPETAG ta_tag = ty_query(ta);
		if(ta_tag == TYFUNC) error("Cannot have function returning function.");
		if(ta_tag == TYARRAY) error("Cannot have function returning array.");
	}
	else {
		ret = get_size_basic(tag);
	}
	
	return ret;
}

unsigned int get_alignment(TYPE t)
{
	unsigned int ret = 0;
	
	TYPETAG tag = ty_query(t);
	if(tag == TYARRAY)
	{
		unsigned int dim;
    	DIMFLAG dimflag;
		TYPE ta = ty_query_array(t, &dimflag, &dim);
		TYPETAG ta_tag = ty_query(ta);
		unsigned int tmp = dim;
		if(ta_tag == TYFUNC) {
			error("Cannot have array of functions.");
			return -1; //could cause errors later
		}
		while(ta_tag == TYARRAY)
		{	
			unsigned int ta_dim;
    		DIMFLAG ta_dimflag;
			ta = ty_query_array(ta, &ta_dimflag, &ta_dim);
			ta_tag = ty_query(ta);
			tmp = tmp * ta_dim;
		}
		unsigned int ta_size = get_size_basic(ta_tag);
		if(tmp*ta_size==0) error("Illegal array dimension.");
		else {
			ret = ta_size;
		}
	}
	else if(tag == TYPTR) {
		ret = 4;
	}
	else if(tag == TYFUNC) {
		PARAMSTYLE paramstyle;
		PARAM_LIST params;
		TYPE ta = ty_query_func(t, &paramstyle, &params);
		TYPETAG ta_tag = ty_query(ta);
		if(ta_tag == TYFUNC) error("Cannot have function returning function.");
		if(ta_tag == TYARRAY) error("Cannot have function returning array.");
	}
	else {
		ret = get_size_basic(tag);
	}
	
	return ret;
}

BOOLEAN duplication_in_param_list(PARAM_LIST list, PARAM_LIST node)
{
	PARAM_LIST p = list;
	PARAM_LIST n = node;
	while(p->next != NULL)
	{
		char * p_id = st_get_id_str(p->id);
		char * n_id = st_get_id_str(n->id);
		if(strcmp(p_id, n_id) == 0)
		{
			error("Duplicate parameter declaration for %s", p_id);
			return TRUE;
		}
		p = p->next;
	}
	return FALSE;
}

void func_install(TYPE t, ST_ID id)
{
	int block;
	ST_DR look_up = st_lookup(id, &block);
	if(look_up == NULL){
		ST_DR rec = stdr_alloc();
		rec->tag = FDECL;
		rec->u.decl.type = t;
		rec->u.decl.sc=NO_SC;
		if (!st_install(id, rec)) {
			error("Duplicate declaration for function %s.", st_get_id_str(id));
			stdr_free(rec);
  		}
	}
	else{
		if(look_up->tag == GDECL) {
			PARAM_LIST params;
			BOOLEAN check_args;
			if(ty_query(look_up->u.decl.type) != TYFUNC)
			{
				error("Incompatible function declaration %s", st_get_id_str(id));
				return;
			}
			TYPE type_exs = ty_query_func_pascal(look_up->u.decl.type,&params,&check_args);
			if((ty_query(look_up->u.decl.type) == TYFUNC) 
				&& (type_exs == t))
			{
				/*ST_DR stdr = stdr_alloc();
				stdr->tag = FDECL;
				stdr->u.decl.type=t;
				stdr->u.decl.sc=NO_SC;
				if (!st_install(id, stdr)) 
				{
					error("Duplicate declaration for %s.", st_get_id_str(id));
					stdr_free(stdr);
				}*/
				look_up->tag = FDECL;
			}
			else error("Typetag is not TYFUNC/return type does not match.");
		}
		else if(look_up->tag == FDECL)
			error("Duplicate function definition %s", st_get_id_str(id));
		else
			error("Tag does not correspond to function.");
	}
}

void unary_conv(TYPETAG * tag)
{
	if(*tag == TYSIGNEDCHAR || *tag == TYSIGNEDSHORTINT)
	{
		b_convert(*tag, TYSIGNEDINT);
		*tag = TYSIGNEDINT;
	}
	if(*tag == TYFLOAT)
	{
		b_convert(*tag, TYDOUBLE);
		*tag = TYDOUBLE;
	}
	if(*tag == TYARRAY)
	{
		b_convert(*tag, TYPTR);
		*tag = TYPTR;
	}
	/*if(*tag == TYFUNC)
	{
		b_convert(*tag, TYPTR);
		*tag = TYPTR;
	}*/
}


void binary_conv_left(TYPETAG * left, TYPETAG right)
{
	if(*left == TYSIGNEDINT && right == TYDOUBLE)
	{
		b_convert(TYSIGNEDINT, TYDOUBLE);
		*left = TYDOUBLE;
	}
}

void binary_conv_right(TYPETAG left, TYPETAG * right)
{
	if(left == TYDOUBLE && *right == TYSIGNEDINT)
	{
		b_convert(TYSIGNEDINT, TYDOUBLE);
		*right = TYDOUBLE;
	}
}

void evaluate(ENODE expr)
{
	//error("begin evaluation");
	EXPR_TYPE expr_type = expr->expr_type;
	switch(expr_type)
	{
		case(INTCONST) :
		{
			//error("an int constant");
			b_push_const_int(expr->u_expr.intval);
			break;
		}
		case(FPCONST) :
		{
			//error("a floating point constant");
			b_push_const_double(expr->u_expr.doubleval);
			break;
		}
		case(ID) :
		{
			//error("an ID");
			b_push_ext_addr(st_get_id_str(expr->u_expr.id));
			break;
		}
		case(UNOP) :
		{
			//error("a unary op");
			evaluate(expr->u_expr.unop.arg);
			UNOP_TYPE unop = expr->u_expr.unop.op;
			TYPETAG tag = ty_query(expr->type);
			b_deref(tag);
			unary_conv(&tag);
			//if(unop = UMINUS)
				b_negate(tag);
			break;
		}
		case(ASSOP) :
		{
			//error("an assignment");
			eval_binop_assop(expr);
			break;
		}
		case(BINOP) :
		{
			//error("a binary op");	
			eval_binop_binop(expr);
			break;	
		}
		case(COMP)	:
		{
			//error("a comparison");
			eval_binop_comp(expr);
			break;
		}
		case(FCALL) :
		{
			//error("a function call");
			int count = 0;
			EXPR_LIST expr_list = expr->u_expr.fcall.actual_args;
			if(expr_list == NULL)
			{
				count = 0;
			}
			else
			{
				count++;				
				while(expr_list->next != NULL) 
				{
					count++;
				}
			}
			b_alloc_arglist(count*8);
			char * fname = st_get_id_str(expr->u_expr.fcall.fname->u_expr.id);
			PARAM_LIST params;
			BOOLEAN check_args;
			TYPE type;
			if(ty_query(expr->u_expr.fcall.fname->type) == TYFUNC) 
				type = ty_query_func_pascal(expr->u_expr.fcall.fname->type,&params,&check_args);
			else type = ty_query_func_pascal(expr->type,&params,&check_args);
			b_funcall_by_name(fname, ty_query(type));
			break;
		}
	}
	//error("end evaluation");
}

void eval_binop_assop(ENODE expr)
{
	if(expr->type == NULL) return;
	ASSOP_TYPE binop = expr->u_expr.assop.op;
	TYPETAG tag = ty_query(expr->type);
	ENODE left_arg = expr->u_expr.assop.left;
	ENODE right_arg = expr->u_expr.assop.right;
	if(left_arg->type == NULL) return;
	if(right_arg->type == NULL) return;
	TYPETAG left_typetag = ty_query(left_arg->type);
	TYPETAG right_typetag = ty_query(right_arg->type);

	//error("assop left type: %d",left_typetag);
	//error("assop right type: %d",right_typetag);	
	
	/*if(left_arg->expr_type == ID && binop != EQUALS)
	{
		evaluate(left_arg);
		if(left_arg->type == NULL) return;
		b_deref(ty_query(left_arg->type));
	}
	else*/ evaluate(left_arg);
	left_typetag = ty_query(left_arg->type);
	
	//if(binop != EQUALS) unary_conv(&left_typetag);
	
	//if(binop != EQUALS) binary_conv_left(&left_typetag, right_typetag);
	
	if(right_arg->expr_type==ID) 
	{
		if(right_arg->type == NULL) return;
		evaluate(right_arg);
		b_deref(ty_query(right_arg->type));
	}
	/*else if(right_arg->expr_type == UNOP)
	{
		if(right_arg->u_expr.unop.arg->expr_type == ID)
			b_push_ext_addr(st_get_id_str(right_arg->u_expr.unop.arg->u_expr.id));
		evaluate(right_arg);
	}*/
	/*else if(right_arg->expr_type == FCALL && left_arg->expr_type != FCALL)
	{
		PARAM_LIST params;
		BOOLEAN check_args;
		TYPE type;
		if(left_typetag != ty_query(right_arg->u_expr.fcall.fname->type))
		{
			type = ty_query_func_pascal(right_arg->type,&params,&check_args);
			evaluate(right_arg);
			b_convert(ty_query(type), left_typetag);
		}
		else
			evaluate(right_arg);
	}*/
	else evaluate(right_arg);
	right_typetag = ty_query(right_arg->type);
	
	//if(binop != EQUALS) unary_conv(&right_typetag);
	
	//if(binop != EQUALS) binary_conv_right(left_typetag, &right_typetag);

	/*if(right_arg->expr_type == BINOP)
	{
		b_assign(left_typetag);
		return;
	}*/			
	
	if(left_arg->expr_type == FCALL || right_arg->expr_type == FCALL)
	{
		PARAM_LIST params;
		BOOLEAN check_args;
		if(left_typetag == TYFUNC)
		{
			TYPE type;
			if(ty_query(left_arg->u_expr.fcall.fname->type) == TYFUNC) 
				type = ty_query_func_pascal(left_arg->u_expr.fcall.fname->type,&params,&check_args);
			else type = ty_query_func_pascal(left_arg->type,&params,&check_args);
			left_typetag = ty_query(type);
		}
		if(right_typetag == TYFUNC)
		{
			TYPE type;
			if(ty_query(right_arg->u_expr.fcall.fname->type) == TYFUNC) 
				type = ty_query_func_pascal(right_arg->u_expr.fcall.fname->type,&params,&check_args);
			else type = ty_query_func_pascal(right_arg->type,&params,&check_args);
			right_typetag = ty_query(type);
		}
	}
	
	/*if(binop == EQUALS)
	{*/
		TYPETAG my_typetag = ty_query(expr->type);
		if((left_typetag != right_typetag) && (right_arg->expr_type != FCALL))
		{
			//char * id_left = st_get_id_str(left_arg->u_expr.id);
			//char * id_right = st_get_id_str(right_arg->u_expr.id);
			//error("left: %s = %d",id_left, left_typetag);
			//error("right: %s = %d",id_right, right_arg->expr_type);
			//error("before left: %d", left_typetag);
			//error("before right: %d", right_typetag);
			//error("after me: %d", my_typetag);
			//error("unequaltypeset");
			if(left_typetag == TYFLOAT && right_typetag == TYSIGNEDINT
				&& right_arg->expr_type == BINOP
				&& right_arg->u_expr.binop.op == PLUS)
			{
				//error("her");
				b_convert(TYDOUBLE, TYFLOAT);
			}
			else
			{
//b_lineno_comment (5555);
			//error("hello");
			//unary_conv(&right_typetag);
			if(left_typetag == TYDOUBLE && right_typetag == TYSIGNEDINT)
			{
				b_convert(TYSIGNEDINT, TYDOUBLE);
				right_typetag = TYDOUBLE;
			}
			if(left_typetag == TYFLOAT && right_typetag == TYSIGNEDINT)
			{
				b_convert(TYSIGNEDINT, TYFLOAT);
				right_typetag = TYFLOAT;
			}
			if(left_typetag == TYFLOAT && right_typetag == TYDOUBLE)
			{
				b_convert(TYDOUBLE, TYFLOAT);
				right_typetag = TYFLOAT;
			}

			if(left_typetag == TYSIGNEDINT && right_typetag == TYDOUBLE)
			{
				b_convert(TYDOUBLE, TYSIGNEDINT);
				right_typetag = TYSIGNEDINT;
			}
			//b_convert(right_typetag, left_typetag);
		//right_typetag = left_typetag;
		//expr->type = left_arg->type;
			//right_arg->type = left_arg->type;
			//b_convert(left_typetag, right_typetag);
			}
		}

		
		//error("after left: %d", left_typetag);
		//error("after right: %d", right_typetag);
		//error("after me2: %d", my_typetag);
		
		if(left_arg->expr_type == ID)
		{
			//error("setid");
			int block;
			ST_DR stdr = st_lookup(left_arg->u_expr.id, &block);
			if(stdr != NULL && stdr->tag == FDECL)
			{
				error("left side of assignment is not l-value");
				return;
			}
		}
					
		b_assign(left_typetag);

	/*}
	else
	{
		if(right_typetag == TYFLOAT) bug("Right typetag is float");
		if(right_typetag != left_typetag) 
		{
		}
		else tag = right_typetag;
		
		B_ARITH_REL_OP barop;
		if(binop == PLUS) barop = B_ADD;
		if(binop == MINUS) barop = B_SUB;
		if(binop == TIMES) barop = B_MULT;
		if(binop == DIVIDE) barop = B_DIV;
		//if(binop == BIN_MOD) barop = B_MOD;
		if(binop == LESS_THAN) barop = B_LT;
		if(binop == LESS_THAN_EQUAL) barop = B_LE;
		if(binop == GREATER_THAN) barop = B_GT;
		if(binop == GREATER_THAN_EQUAL) barop = B_GE;
		if(binop == NOT_EQUAL) barop = B_NE;
		if(binop == DOUBLE_EQUALS) barop = B_EQ;
		b_arith_rel_op(barop, tag);
		right_typetag = left_typetag;
		expr->u_expr.binop.right->type = left_arg->type;	
	}*/
}

void eval_binop_binop(ENODE expr)
{
	BINOP_TYPE binop = expr->u_expr.binop.op;
	TYPETAG tag = ty_query(expr->type);
	ENODE left_arg = expr->u_expr.binop.left;
	ENODE right_arg = expr->u_expr.binop.right;	
	
	TYPETAG left_typetag = ty_query(left_arg->type);
	TYPETAG right_typetag = ty_query(right_arg->type);	

	//error("before left: %d", left_typetag);
	//error("before right: %d", right_typetag);
	
	if(left_arg->expr_type == ID)
	{
		evaluate(left_arg);
		if(left_arg->type == NULL) return;
		b_deref(ty_query(left_arg->type));
	}
	else evaluate(left_arg);
	
	//unary_conv(&left_typetag);
	
	//binary_conv_left(&left_typetag, right_typetag);
	//here	
	unary_conv(&left_typetag);
if(left_typetag != right_typetag)
	{
		//error("notequaltypes");
		if((left_typetag == TYSIGNEDCHAR || left_typetag == TYSIGNEDSHORTINT) && right_typetag == TYSIGNEDINT)
		{
			b_convert(left_typetag, TYSIGNEDINT);
			left_typetag = TYSIGNEDINT;
		}
		/*else if((left_typetag == TYSIGNEDCHAR || left_typetag == TYSIGNEDSHORTINT) && right_typetag == TYFLOAT)
		{
			b_convert(left_typetag, TYFLOAT);
			left_typetag = TYFLOAT;
		}*/
		else if((left_typetag == TYSIGNEDCHAR || left_typetag == TYSIGNEDSHORTINT) && right_typetag == TYDOUBLE)
		{
			b_convert(left_typetag, TYDOUBLE);
			left_typetag = TYDOUBLE;
		}
		else if(left_typetag == TYSIGNEDINT && right_typetag == TYFLOAT)
		{
			b_convert(left_typetag, TYDOUBLE);
			left_typetag = TYDOUBLE;
		}
		else if(left_typetag == TYSIGNEDINT && right_typetag == TYDOUBLE)
		{
			b_convert(left_typetag, TYDOUBLE);
			left_typetag = TYDOUBLE;
		}
		else if(left_typetag == TYFLOAT && right_typetag == TYDOUBLE)
		{
			b_convert(left_typetag, TYDOUBLE);
			left_typetag = TYDOUBLE;
		}
	}

	if(right_arg->expr_type==ID) 
	{
		evaluate(right_arg);
		b_deref(ty_query(right_arg->type));
	}
	/*else if(right_arg->expr_type == UNOP && right_arg->u_expr.unop.op == UMINUS)
	{
		if(right_arg->u_expr.unop.arg->expr_type == ID)
			b_push_ext_addr(st_get_id_str(right_arg->u_expr.unop.arg->u_expr.id));
		evaluate(right_arg);
	}*/
	/*else if(right_arg->expr_type == FCALL && left_arg->expr_type != FCALL)
	{
		PARAM_LIST params;
		BOOLEAN check_args;
		TYPE type;
		if(left_typetag != ty_query(right_arg->u_expr.fcall.fname->type))
		{
			type = ty_query_func_pascal(right_arg->type,&params,&check_args);
			evaluate(right_arg);
			b_convert(ty_query(type), left_typetag);
		}
		else
			evaluate(right_arg);
	}*/
	else evaluate(right_arg);

	
	//unary_conv(&right_typetag);

	//error("before left: %d", left_typetag);
	//error("before right: %d", right_typetag);
unary_conv(&right_typetag);
	
	//binary_conv_right(left_typetag, &right_typetag);
	//binary_conv_left(&left_typetag, right_typetag);

	/*if(binop == EQUALS && right_arg->expr_type == BINOP 
		&& right_arg->u_expr.binop.op == EQUALS)
	{
		b_assign(left_typetag);
		return;
	}*/			
	
	if(left_arg->expr_type == FCALL || right_arg->expr_type == FCALL)
	{
		PARAM_LIST params;
		BOOLEAN check_args;
		if(left_typetag == TYFUNC)
		{
			TYPE type;
			if(ty_query(left_arg->u_expr.fcall.fname->type) == TYFUNC) 
				type = ty_query_func_pascal(left_arg->u_expr.fcall.fname->type,&params,&check_args);
			else type = ty_query_func_pascal(left_arg->type,&params,&check_args);
			left_typetag = ty_query(type);
		}
		if(right_typetag == TYFUNC)
		{
			TYPE type;
			if(ty_query(right_arg->u_expr.fcall.fname->type) == TYFUNC) 
				type = ty_query_func_pascal(right_arg->u_expr.fcall.fname->type,&params,&check_args);
			else type = ty_query_func_pascal(right_arg->type,&params,&check_args);
			right_typetag = ty_query(type);
		}
	}


	//unary_conv(&left_typetag);
	unary_conv(&right_typetag);
	if(left_typetag != right_typetag)
	{
		//error("notequaltypes");
		
		if((right_typetag == TYSIGNEDCHAR || right_typetag == TYSIGNEDSHORTINT) && left_typetag == TYSIGNEDINT)
		{
			b_convert(right_typetag, TYSIGNEDINT);
			right_typetag = TYSIGNEDINT;
		}
		/*else if((right_typetag == TYSIGNEDCHAR || right_typetag == TYSIGNEDSHORTINT) && left_typetag == TYFLOAT)
		{
			b_convert(right_typetag, TYFLOAT);
			right_typetag = TYFLOAT;
		}*/
		else if((right_typetag == TYSIGNEDCHAR || right_typetag == TYSIGNEDSHORTINT) && left_typetag == TYDOUBLE)
		{
			b_convert(right_typetag, TYDOUBLE);
			right_typetag = TYDOUBLE;
		}
		/*else if(right_typetag == TYSIGNEDINT && left_typetag == TYFLOAT)
		{
			b_convert(right_typetag, TYFLOAT);
			right_typetag = TYFLOAT;
		}*/
		else if(right_typetag == TYSIGNEDINT && left_typetag == TYDOUBLE)
		{
			b_convert(right_typetag, TYDOUBLE);
			right_typetag = TYDOUBLE;
		}
		else if(right_typetag == TYFLOAT && left_typetag == TYDOUBLE)
		{
			b_convert(right_typetag, TYDOUBLE);
			right_typetag = TYDOUBLE;
		}
	}
	
	/*if(binop == EQUALS)
	{
		if(left_typetag != right_typetag && right_arg->expr_type != FCALL) 
		{
			if(left_typetag == TYFLOAT && right_typetag == TYSIGNEDINT
				&& right_arg->expr_type == BINOP
				&& right_arg->u_expr.binop.op == PLUS)
			{
				b_convert(TYDOUBLE, TYFLOAT);
			}
			else
			{
			unary_conv(&right_typetag);
			b_convert(right_typetag, left_typetag);
			}
		}			
		b_assign(left_typetag);
	}
	else
	{*/
	//error("after left: %d", left_typetag);
	//error("after right: %d", right_typetag);
	if(left_typetag != right_typetag)
	{
		//error("Not Equal");
	}
		if(right_typetag == TYFLOAT) bug("Right typetag is float");
		if(right_typetag != left_typetag) 
		{
		}
		else tag = right_typetag;
		//error("sedning up tag: %d", tag);
		
		B_ARITH_REL_OP barop;
		if(binop == PLUS) barop = B_ADD;
		if(binop == MINUS) barop = B_SUB;
		if(binop == TIMES) barop = B_MULT;
		if(binop == DIVIDE) barop = B_DIV;
		//if(binop == BIN_MOD) barop = B_MOD;
		//if(binop == LESS_THAN) barop = B_LT;
		//if(binop == LESS_THAN_EQUAL) barop = B_LE;
		//if(binop == GREATER_THAN) barop = B_GT;
		//if(binop == GREATER_THAN_EQUAL) barop = B_GE;
		//if(binop == NOT_EQUAL) barop = B_NE;
		//if(binop == DOUBLE_EQUALS) barop = B_EQ;
		b_arith_rel_op(barop, tag);
		right_typetag = left_typetag;
		expr->u_expr.binop.right->type = left_arg->type;
		expr->type = ty_build_basic(tag);
	//}	
}

void eval_binop_comp(ENODE expr)
{
	COMP_TYPE binop = expr->u_expr.comp.op;
	TYPETAG tag = ty_query(expr->type);
	ENODE left_arg = expr->u_expr.comp.left;
	ENODE right_arg = expr->u_expr.comp.right;	
	
	TYPETAG left_typetag = ty_query(left_arg->type);
	TYPETAG right_typetag = ty_query(right_arg->type);	
	
	if(left_arg->expr_type == ID && binop != EQUALS)
	{
		evaluate(left_arg);
		if(left_arg->type == NULL) return;
		b_deref(ty_query(left_arg->type));
	}
	else evaluate(left_arg);
	
	if(binop != EQUALS) unary_conv(&left_typetag);
	
	if(binop != EQUALS) binary_conv_left(&left_typetag, right_typetag);
	
	if(right_arg->expr_type==ID) 
	{
		evaluate(right_arg);
		b_deref(ty_query(right_arg->type));
	}
	else if(right_arg->expr_type == UNOP && right_arg->u_expr.unop.op == UMINUS)
	{
		if(right_arg->u_expr.unop.arg->expr_type == ID)
			b_push_ext_addr(st_get_id_str(right_arg->u_expr.unop.arg->u_expr.id));
		evaluate(right_arg);
	}
	else if(right_arg->expr_type == FCALL && left_arg->expr_type != FCALL && binop == EQUALS)
	{
		PARAM_LIST params;
		BOOLEAN check_args;
		TYPE type;
		if(left_typetag != ty_query(right_arg->u_expr.fcall.fname->type))
		{
			type = ty_query_func_pascal(right_arg->type,&params,&check_args);
			evaluate(right_arg);
			b_convert(ty_query(type), left_typetag);
		}
		else
			evaluate(right_arg);
	}
	else evaluate(right_arg);
	
	if(binop != EQUALS) unary_conv(&right_typetag);
	
	if(binop != EQUALS) binary_conv_right(left_typetag, &right_typetag);

	if(binop == EQUALS && right_arg->expr_type == BINOP 
		&& right_arg->u_expr.binop.op == EQUALS)
	{
		b_assign(left_typetag);
		return;
	}			
	
	if(left_arg->expr_type == FCALL || right_arg->expr_type == FCALL)
	{
		PARAM_LIST params;
		BOOLEAN check_args;
		if(left_typetag == TYFUNC)
		{
			TYPE type;
			if(ty_query(left_arg->u_expr.fcall.fname->type) == TYFUNC) 
				type = ty_query_func_pascal(left_arg->u_expr.fcall.fname->type,&params,&check_args);
			else type = ty_query_func_pascal(left_arg->type,&params,&check_args);
			left_typetag = ty_query(type);
		}
		if(right_typetag == TYFUNC)
		{
			TYPE type;
			if(ty_query(right_arg->u_expr.fcall.fname->type) == TYFUNC) 
				type = ty_query_func_pascal(right_arg->u_expr.fcall.fname->type,&params,&check_args);
			else type = ty_query_func_pascal(right_arg->type,&params,&check_args);
			right_typetag = ty_query(type);
		}
	}
	
	if(binop == EQUALS)
	{
		if(left_typetag != right_typetag && right_arg->expr_type != FCALL) 
		{
			if(left_typetag == TYFLOAT && right_typetag == TYSIGNEDINT
				&& right_arg->expr_type == BINOP
				&& right_arg->u_expr.binop.op == PLUS)
			{
				b_convert(TYDOUBLE, TYFLOAT);
			}
			else
			{
			unary_conv(&right_typetag);
			b_convert(right_typetag, left_typetag);
			}
		}			
		b_assign(left_typetag);
	}
	else
	{
		if(right_typetag == TYFLOAT) bug("Right typetag is float");
		if(right_typetag != left_typetag) 
		{
		}
		else tag = right_typetag;
		
		B_ARITH_REL_OP barop;
		if(binop == PLUS) barop = B_ADD;
		if(binop == MINUS) barop = B_SUB;
		if(binop == TIMES) barop = B_MULT;
		if(binop == DIVIDE) barop = B_DIV;
		//if(binop == BIN_MOD) barop = B_MOD;
		if(binop == LESS_THAN) barop = B_LT;
		if(binop == LESS_THAN_EQUAL) barop = B_LE;
		if(binop == GREATER_THAN) barop = B_GT;
		if(binop == GREATER_THAN_EQUAL) barop = B_GE;
		if(binop == NOT_EQUAL) barop = B_NE;
		if(binop == DOUBLE_EQUALS) barop = B_EQ;
		b_arith_rel_op(barop, tag);
		right_typetag = left_typetag;
		expr->u_expr.binop.right->type = left_arg->type;	
	}
}




