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
		if(look_up->tag == GDECL && look_up->u.decl.type == t) {
			look_up->tag = FDECL;
		}
		else {
			error("Not function or wrong return type %s.", st_get_id_str(id));
		}
	}
}






