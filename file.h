#ifndef FILE_H
#define FILE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "def.h"

typedef struct _file {
  FILE *fp;
  size_t size;
  size_t segment_size;
  size_t block_size;
  int nsegs;
} File;

void get_read_fp(File *f, char *filename);
void get_write_fp(File *f, char *filename);
void prepare_download_file(File *f, char *filename, size_t filesize);
void init_file(File *f, size_t segment_size, size_t block_size);

size_t get_num_segments(File *f);
size_t get_num_blocks(File *f, size_t seg_idx); 
size_t get_segment_size(File *f, size_t s_idx);

size_t read_file(File *f, char *buffer, size_t num_bytes, size_t offset);
size_t write_file(File *f, char *buffer, size_t num_bytes, size_t offset);

size_t get_segment(File *file_info, char *buffer, size_t idx);
void   write_segment(File *file_info, char *buffer, size_t idx);

size_t get_block(File *f, char *buffer, size_t s_idx, size_t b_idx);
void   write_block(File *f, char *buffer, size_t s_idx, size_t b_idx);


#endif
