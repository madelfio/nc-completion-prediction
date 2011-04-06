#include "def.h"
#include "bitfield.h"

unsigned char BIT_HEX[] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};

void init_bf(BitField *bf, size_t npcs) {
  DLOG("Init'ing bitfield with %zu pieces\n",npcs);
  bf->nbits = npcs;
  bf->nbytes = bf->nbits / 8;
  if (bf->nbits % 8) bf->nbytes++;

  bf->b = (unsigned char *)malloc(bf->nbytes);
  if (!(bf->b)) {
    printf("error init'ing bitfield");
    exit(1);
  }

  // clear bitfield
  int i;
  for (i = 0; i < bf->nbytes; i++) {
    bf->b[i] = 0;
  }

  bf->nset = 0;
  DNOTE();
}

void del_bf(BitField *bf) {
  if (bf->b) {
    free(bf->b);
  }
}

void set_bf(BitField *bf, size_t idx) {
  if (idx > bf->nbits) {
    return;
  }

  if (!isset_bf(bf, idx)) {
    bf->b[idx / 8] |= BIT_HEX[idx % 8];
    bf->nset++;
  } else {
    printf("trying to reset segment %zu. WHY?\n", idx);
  }
}

void unset_bf(BitField *bf, size_t idx) {
  if (idx > bf->nbits) {
    return;
  }

  if (isset_bf(bf, idx)) {
    bf->b[idx / 8] &= (~BIT_HEX[idx % 8]);
    bf->nset--;
  }
}

int isset_bf(BitField *bf, size_t idx) {
  if (idx > bf->nbits) {
    return 0;
  }
  return ((bf->b[idx / 8] & BIT_HEX[idx % 8]) > 0);
}

size_t first_empty(BitField *bf) {
  size_t i;
#ifdef DEBUG
  for (i = 0; i < bf->nbits; i++) {
    printf("%d", isset_bf(bf, i));
  }
  printf("\n");
#endif
  for (i = 0; i < bf->nbits; i++) {
    if (!isset_bf(bf, i)) {
      return i;
    }
  }
  //printf("seems to be full... nset: %zu, nbits: %zu\n", 
  //    bf->nset, bf->nbits);
  return -1;
}

size_t next_empty(BitField *bf, size_t start) {
  size_t i;
  for (i = start; i < bf->nbits; i++) {
    if (!isset_bf(bf, i)) {
      return i;
    }
  }
  return -1;
}

int isfull_bf(BitField *bf) {
  DLOG("%zu >= %zu ??\n", bf->nset, bf->nbits);
  return bf->nset >= bf->nbits;
}

size_t numbytes_bf(BitField *bf) {
  return bf->nbytes;
}

size_t numbits_bf(BitField *bf) {
  return bf->nbits;
}

size_t count_bf(BitField *bf) {
  return bf->nset;
}
