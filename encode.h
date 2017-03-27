/*
* definitions to support encode.c
*/

#ifndef ENCODE_H
#define ENCODE_H

#include "types.h"
#include "message.h"

unsigned int get_size(TYPE t);
unsigned int get_alignment(TYPE t);

#endif
