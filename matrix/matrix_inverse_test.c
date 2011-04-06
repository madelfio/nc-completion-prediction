#include <assert.h>
#include <stdio.h>
#include "gf_math.h"
#include "matrix_inverse.h"

static void test_simple_row() {
  byte a[1][2] = {{1,5}};
  decode_matrix(&a[0][0],1,1);
  assert( a[0][0] == 1 &&
          a[0][1] == 5 &&
          "test_simple_row");
}


static void test_two_rows() {
  byte a[2][3] = {{1,0,1},
                  {0,1,2}};
  decode_matrix(&a[0][0],2,1);
  assert( a[0][0] == 1 &&
          a[0][1] == 0 &&
          a[0][2] == 1 &&
          a[1][0] == 0 &&
          a[1][1] == 1 &&
          a[1][2] == 2 &&
          "test_two_rows");
}

static void test_inverted_rows() {
  byte a[2][3] = {{0,1,2},
                  {1,0,1}};
  decode_matrix(&a[0][0],2,1);
  assert( a[0][0] == 1 &&
          a[0][1] == 0 &&
          a[0][2] == 1 &&
          a[1][0] == 0 &&
          a[1][1] == 1 &&
          a[1][2] == 2 &&
          "test_two_rows");
}

static void test_simple_combination() {
  byte a[2][3] = {{1,1,3},
                  {0,1,2}};
  decode_matrix(&a[0][0],2,1);
  assert(a[0][0] == 1 &&
         a[0][1] == 0 &&
         a[0][2] == 1 &&
         a[1][0] == 0 &&
         a[1][1] == 1 &&
         a[1][2] == 2 &&
         "test_simple_combination");
}

static void test_coefficient_generator() {
  byte coeff[3];
  get_rand_bytes(&coeff[0], 3);
  printf("%d,%d,%d\n", coeff[0],coeff[1],coeff[2]);
  assert(coeff[0] + coeff[1] + coeff[2] > 0 &&
         "test_coefficient_generator");
}

static void test_rand_matrix() {
  byte vals[2];
  get_rand_bytes(&vals[0],2);
  byte coeff[2][3];
  get_rand_bytes(&coeff[0][0], 2);
  get_rand_bytes(&coeff[1][0], 2);
  coeff[0][2] = Sum(Product(vals[0],coeff[0][0]),
                    Product(vals[1],coeff[0][1]));
  coeff[1][2] = Sum(Product(vals[0],coeff[1][0]),
                    Product(vals[1],coeff[1][1]));
  printf("vals: %d, %d\n", vals[0], vals[1]);
  printf("matrix: %d, %d, %d\n", coeff[0][0], coeff[0][1], coeff[0][2]);
  printf("        %d, %d, %d\n", coeff[1][0], coeff[1][1], coeff[1][2]);
  decode_matrix(&coeff[0][0],2,1);
  printf("matrix: %d, %d, %d\n", coeff[0][0], coeff[0][1], coeff[0][2]);
  printf("        %d, %d, %d\n\n", coeff[1][0], coeff[1][1], coeff[1][2]);
  assert(coeff[0][2] == vals[0] &&
         coeff[1][2] == vals[1] &&
         "test_rand_matrix");
}

static void test_encoding() {
  char *message = "Hello, my name is Marco.";
}


int main() {
  gf_init();
  test_simple_row();
  test_two_rows();
  test_inverted_rows();
  test_simple_combination();
  test_coefficient_generator();
  test_rand_matrix();
  // byte a[4][5] = {{10,10,10,10,/**/60},
  //                 {10,10, 0,10,/**/30},
  //                 {10, 0,10, 5,/**/50},
  //                 {3,  0, 4, 1,/**/100}};
  // byte b[4][5] = {{0,2,0,0,/**/30},
  //                 {0,0,0,5,/**/10},
  //                 {1,0,0,0,/**/5},
  //                 {0,0,1,0,/**/6}};
  //decode_matrix(&a[0][0],4,1);
  return 0;
}
