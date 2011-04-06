#ifndef SEGMENT_H
#define SEGMENT_H

#include <stdlib.h>
#include <stdio.h>

#include "matrix/gf_math.h"
#include "matrix/matrix_inverse.h"

typedef struct _segment {
  byte *matrix;
  int n; // Blocks per segment
  int k; // Block length
  int deg_of_freedom;
  int status;

  // use for predictive braking
  int last_dof;
  int last_server_cnt;
  struct timeval last_time;
  int est_server_cnt;
} Segment;

void clear_segment(Segment *seg);
void init_segment(Segment *seg, int n, int k);
int is_uninitialized(Segment *seg);
int is_active(Segment *seg);
int is_done(Segment *seg);
int dof(Segment *seg);
void free_segment(Segment *seg);
int add_new_packet(Segment *seg, byte *msg);
int is_segment_done(Segment *seg);
void get_decoded_segment(Segment *seg, char *buf);

void init_est(Segment *seg, int server_cnt, struct timeval now);
int update_stats(Segment *seg, int server_cnt, struct timeval now);
int get_est(Segment *seg);

#endif
