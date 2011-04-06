#ifndef GFMATH_H
#define GFMATH_H
typedef unsigned char byte;

void gf_init();

byte Sum(byte a, byte b);
byte Diff(byte a, byte b);
byte Product(byte a, byte b);
byte Quotient(byte a, byte b);
byte Inverse(byte a);

void get_rand_bytes(byte *coeff, int n);
void get_rand_bytes_s(byte *coeff, int n, int seed);
#endif
