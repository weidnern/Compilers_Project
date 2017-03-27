/*
* encode.c
*
* This class contains the code generation routines.
*/

#include "encode.h"
#include "types.h"
#include "message.h"

int get_size(TYPE t)
{
	int ret = 0;
	
	TYPETAG tag = ty_query(t);
	if(tag == TYARRAY)
	{
		int dim;
    	DIMFLAG dimflag;
		TYPE ta = ty_query_array(t, &dimflag, &dim);
		TYPETAG ta_tag = ty_query(ta);
		int tmp = dim;
		if(ta_tag == TYFUNC) {
			error("Cannot have array of functions.");
		}
		while(ta_tag == TYARRAY)
		{	
			int ta_dim;
    		DIMFLAG ta_dimflag;
			ta = ty_query_array(ta, &ta_dimflag, &ta_dim);
			ta_tag = ty_query(ta);
			tmp = tmp * ta_dim;
		}
		int ta_size = get_size_basic(ta_tag);
		if(tmp*ta_size==0) error("Illegal array dimension.");
		else {
			ret = tmp * ta_size;
		}
	}
	else if(tag == TYPTR) {
		ret = 4;
	}
	else {
		ret = get_size_basic(tag);
	}
	
	return ret;
}

int get_alignment(TYPE t)
{
	int ret = 0;
	
	TYPETAG tag = ty_query(t);
	if(tag == TYARRAY)
	{
		int dim;
    	DIMFLAG dimflag;
		TYPE ta = ty_query_array(t, &dimflag, &dim);
		TYPETAG ta_tag = ty_query(ta);
		int tmp = dim;
		if(ta_tag == TYFUNC) {
			error("Cannot have array of functions.");
		}
		while(ta_tag == TYARRAY)
		{	
			int ta_dim;
    		DIMFLAG ta_dimflag;
			ta = ty_query_array(ta, &ta_dimflag, &ta_dim);
			ta_tag = ty_query(ta);
			tmp = tmp * ta_dim;
		}
		int ta_size = get_size_basic(ta_tag);
		if(tmp*ta_size==0) error("Illegal array dimension.");
		else {
			ret = ta_size;
		}
	}
	else if(tag == TYPTR) {
		ret = 4;
	}
	else {
		ret = get_size_basic(tag);
	}
	
	return ret;
}
