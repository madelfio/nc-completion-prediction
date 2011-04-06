#include <string.h>

#include "def.h"
#include "segment.h"

#define UNINITIALIZED 0
#define IN_PROGRESS 1
#define DONE 2

void clear_segment(Segment *seg) {
  seg->n = 0;
  seg->k = 0;
  seg->deg_of_freedom = 0;
  seg->status = UNINITIALIZED;
  seg->last_dof = 0;
}

void init_segment(Segment *seg, int n, int k) {
  DNOTE();
  if (is_active(seg)) {return;}
  DNOTE();
  seg->n = n;
  seg->k = k;
  seg->deg_of_freedom = n;
  DNOTE();
  seg->matrix = (byte *)calloc(sizeof(byte), (n+k)*n);
  if (!seg->matrix) {
    perror("init_segment");
  }
  DNOTE();
  seg->status = IN_PROGRESS;
}

int is_uninitialized(Segment *seg) {
  return (seg->status == UNINITIALIZED);
}

int is_active(Segment *seg) {
  return (seg->status == IN_PROGRESS);
}

int is_done(Segment *seg) {
  return (seg->status == DONE);
}

int dof(Segment *seg) {
  return seg->deg_of_freedom;
}

void free_segment(Segment *seg) {
  free(seg->matrix);
  seg->status = DONE;
}

// return value is 1 if packet is linearly independent
int add_new_packet(Segment *seg, byte *msg) {
  int lin_ind = add_decode_row(seg->matrix, seg->n - seg->deg_of_freedom,
      msg, seg->n, seg->k);
  seg->deg_of_freedom -= lin_ind;

  //printf("Matrix:\n");
  //print_matrix(seg->matrix, seg->n, seg->n + seg->k);
  
  return lin_ind; 
}

int is_segment_done(Segment *seg) {
  return ((seg->status == DONE) ||
      ((seg->status == IN_PROGRESS) && (seg->deg_of_freedom == 0)));
}

void get_decoded_segment(Segment *seg, char *buf) {
  if (!is_segment_done(seg)) {
    printf("Segment is not done!\n");
    exit(1);
  }
  int i;
  for (i = 0; i < seg->n; i++) {
    strncpy(&(buf[seg->k*i]), 
        (char *)&(seg->matrix[(seg->n+seg->k)*i + seg->n]), 
        seg->k);
  }
}

void init_est(Segment *seg, int server_cnt, struct timeval now) {
  seg->last_dof = seg->deg_of_freedom;
  seg->last_server_cnt = server_cnt;
  seg->last_time = now;
  seg->est_server_cnt = -1;
}

int update_stats(Segment *seg, int server_cnt, struct timeval now) {
  int new_est;
  int delta_sc = (server_cnt - seg->last_server_cnt);
  int delta_dof = -(seg->deg_of_freedom - seg->last_dof);
  DLOG("delta_sc: %d, delta_dof: %d\n", delta_sc, delta_dof);
  if (delta_sc <= 1 || delta_dof <= 1) {return 0;}
  new_est = server_cnt + ((delta_sc * seg->deg_of_freedom) / (delta_dof));
  seg->est_server_cnt = new_est;
  seg->last_server_cnt = server_cnt;
  seg->last_dof = seg->deg_of_freedom;
  seg->last_time = now;
  DLOG("new est: %d\n", new_est);
  return 1;
}

int get_est(Segment *seg) {
  return seg->est_server_cnt;
}
