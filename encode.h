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

unsigned int get_size(TYPE t);
unsigned int get_alignment(TYPE t);
BOOLEAN duplication_in_param_list(PARAM_LIST list, PARAM_LIST node);

#endif
