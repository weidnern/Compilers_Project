/*
* definitions to support encode.c
*/

#ifndef ENCODE_H
#define ENCODE_H

#include "types.h"
#include "message.h"
#include "defs.h"
#include "string.h"
#include "symtab.h"
#include "tree.h"
#include "backend-x86.h"

unsigned int get_size(TYPE t);
unsigned int get_alignment(TYPE t);
BOOLEAN duplication_in_param_list(PARAM_LIST list, PARAM_LIST node);
void func_install(TYPE t, ST_ID id);
void evaluate(ENODE expr);
void eval_binop_assop(ENODE expr);
void eval_binop_binop(ENODE expr);
void eval_binop_comp(ENODE expr);

#endif
