/***********************************
 * Modeled after enhanced ctorrent *
 ***********************************/

#ifndef BITFIELD_H
#define BITFIELD_H

#include <stdlib.h>
#include <stdio.h>

#include "def.h"

typedef struct _bitfield {
  size_t nbits;
  size_t nbytes;
  unsigned char *b;
  size_t nset;
} BitField;

void init_bf(BitField *bf, size_t npcs);
void del_bf(BitField *bf);

void set_bf(BitField *bf, size_t idx);
void unset_bf(BitField *bf, size_t idx);
int isset_bf(BitField *bf, size_t idx);

size_t first_empty(BitField *bf);
size_t next_empty(BitField *bf, size_t start);

int isfull_bf(BitField *bf);

size_t numbytes_bf(BitField *bf);
size_t numbits_bf(BitField *bf);
size_t count_bf(BitField *bf);

#endif

