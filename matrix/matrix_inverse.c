// Function decode_matrix
// Take as inputs: matrix, number of non-zero rows, new row
// Output: updated matrix (updates in place)


#include <stdio.h>
#include "gf_math.h"

void print_matrix(byte *matrix, int rows, int cols) {
  int i, j;
  for (i=0; i<rows; i++) {
    for (j=0; j<cols; j++) {
      printf("%d", matrix[i*(cols)+j]);
      printf(" ");
    }
    printf("\n");
  }
  printf("\n");
}

int add_decode_row(byte *matrix, int nonzero_cnt,
                   byte *new_row, int n, int k) {
  // Takes a (n+k)*n sized matrix as input
  // Where the bottom n-nonzero_cnt rows are empty
  // And inserts the new row in the nonzero_cnt'th row
  // And performs gauss-jordan elimination until the nonzero_cnt+1 rows
  // are in reduced row-echelon form.
  // 
  // Code is loosely based on pseudocode from Shojania & Li (IWQoS 2007)
  //
  // Return 0 if new_row is linearly dependent on earlier rows
  // Otherwise, return 1
 
  int i, j, m, pivot, multiplier;
  // reduce leading coefficient of new row to 0
  for (i=0; i < nonzero_cnt; i++) {
    j = 0;
    while (matrix[i*(n+k)+j] == 0 && j < n) {
      j++;
    }
    multiplier = new_row[j];
    // Add j*row i to new row
    for (m=0; m < n+k; m++) {
      int p = Product(matrix[i*(n+k)+m], multiplier);
      new_row[m] = Sum(new_row[m], p);
    }
  }
  

  // Find leading non-zero coefficient in the new coefficient row (pivot)
  pivot = 0;
  while (new_row[pivot] == 0 && pivot <= n)
    pivot++;
  //printf("pivot: %d\n", pivot);

  // Check for linear independence with existing coefficient rows
  if (pivot >= n) {
    return 0;
  }
  //printf("nonzero_cnt: %d\n", nonzero_cnt);
  //printf("pivot: %d\n", pivot);
  //printf("new_row[pivot]: %d\n", new_row[pivot]);

  // Reduce the leading non-zero entry of the new row to 1, such that the
  // result is in REF (except for the new row)
  multiplier = Inverse(new_row[pivot]);
  //printf("\n%d, %d\n",pivot,multiplier);
  // Add pivot*row i to new row
  for (m=0; m < n+k; m++) {
    //printf("m: %d, new_row[m]: %d, multiplier: %d\n",m,new_row[m],multiplier);
    new_row[m] = Product(new_row[m], multiplier);
  }

  //
  // Reduce the coefficient matrix to the reduced row-echelon form (RREF),
  // except for the new row
  for (i=0; i<nonzero_cnt; i++) {
    multiplier = Diff(0, matrix[i*(n+k)+pivot]);
    // Add multiplier*new_row to row i
    for (m=0; m < n+k; m++) {
      int p = Product(new_row[m], multiplier);
      matrix[i*(n+k)+m] = Sum(matrix[i*(n+k)+m], p);
      //printf("p: %d, new_row: %d \n", p, new_row[m]);
    }
  }

  // Move the new row to the appropriate position among the existing rows,
  // so the matrix is finally in RREF
  i = 0;
  j = 0;
  while (i < nonzero_cnt && j < pivot) {
    if (matrix[i*(n+k)+j] == 0) j++;
    else i++;
  }

  int t = i;
  // move all rows between i and nonzero_cnt down a row, insert new_row
  for (i=nonzero_cnt; i > t; i--) {
    for (j=0; j < n+k; j++) {
      matrix[(i)*(n+k)+j] = matrix[(i-1)*(n+k)+j];
    }
  }

  for (j=0; j < n+k; j++) {
    matrix[t*(n+k) + j] = new_row[j];
  };
  
  return 1;
}


void decode_matrix(byte *matrix, int n, int k) {
  int i, j, a;
  byte new_row[n+k];
  for (i=0; i<n; i++) {
    for (j=0; j < n+k; j++) {
      new_row[j] = matrix[i*(n+k)+j];
    }
    //print_matrix(new_row, 1, n+k);
    a = add_decode_row(matrix, i, new_row, n, k);
  }
  //print_matrix(matrix, n, n+k);
}

void encode_matrix(byte *message, int n, int k) {
  // message = array of bytes (e.g., segment of video)
  // n = packets per segment
  // k = bytes per packet
  // len(message) must equal n*k
  //int i;



}

void get_encoded_row(byte *message, int n, int k, byte *encoded_row) {
  get_rand_bytes(&encoded_row[0], n);
  int i;
  for (i=0;i < n; i++) {
    printf("%d ",encoded_row[i]);
  }
  printf("\n\n");
};

