// GF functions
//
// based on examples here:
// http://www.aimglobal.org/technologies/barcode/Galois_Math.pdf
// 
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "gf_math.h"

#define GF 256
#define PP 301

int Log[GF], Exp[GF];

void fill_log_arrays() {
  // Fill the log and exp arrays with GF 
  int i;
  Log[0] = 1-GF; Exp[0] = 1;
  for (i=1; i<GF; i++) {
    Exp[i] = Exp[i-1] * 2;
    if (Exp[i] >= GF) Exp[i] ^= PP;
    Log[(int)Exp[i]] = i;
  }
//  for (i=0; i<GF; i++) {
//    printf("%d: %d, %d\n", i, Log[i], Exp[i]);
//  }
}

void gf_init() {
  srand(time(NULL));
  fill_log_arrays();
}

byte Sum(byte a, byte b) {return a ^ b;}
byte Diff(byte a, byte b) {return a ^ b;}

byte Product (byte a, byte b) {
  if ((a == 0) || (b == 0)) return (0);
  else return Exp[(Log[(int)a] + Log[(int)b]) % (GF-1)];
}

byte Quotient (byte a, byte b) {
  if ((a == 0) || (b == 0)) return (0);
  else return Exp[(Log[(int)a] + Log[(int)b]) % (GF-1)];
}

byte Inverse(byte a) {
  if (a == 0) {
    printf("divide by 0\n");
    exit(1);
  }
  return Exp[GF-1-Log[(int)a]];
}

byte get_rand_byte() {
  return rand() % GF;
}

void get_rand_bytes(byte *coeff, int n) {
  int i;
  for (i=0; i < n; i++) {
    coeff[i] = get_rand_byte();
  }
}

// ONLY USE FOR TESTING!!!
/*void get_rand_bytes(byte *coeff, int n) {
  int i;
  for (i=0; i < n; i++) {
    coeff[i] = 0;
  }
  coeff[0] = 1;
}*/

void get_rand_bytes_s(byte *coeff, int n, int seed) {
  srand(seed);
  int i;
  for (i=0; i < n; i++) {
    coeff[i] = get_rand_byte();
  }
}
