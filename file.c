#include "file.h"

void get_read_fp(File *f, char *filename) {
  FILE *fp;
  size_t file_size;
  if ((fp = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "file open error\n");
    exit(1);
  }

  fseek(fp, 0, SEEK_END);
  file_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  f->fp = fp;
  f->size = file_size;
}

void get_write_fp(File *f, char *filename) {
  FILE *fp;
  if ((fp = fopen(filename, "w")) == NULL) {
    fprintf(stderr, "file open error\n");
    exit(1);
  }

  f->fp = fp;
  f->size = 0;
}

void prepare_download_file(File *f, char *filename, size_t filesize) {
  get_write_fp(f, filename);
  if (ftruncate(fileno(f->fp), filesize) != 0) {
    perror("file creation error");
    exit(1);
  }
  f->size = filesize;
}

void init_file(File *f, size_t segment_size, size_t block_size) {
  f->segment_size = segment_size;
  f->block_size = block_size;
  f->nsegs = f->size / segment_size;
  if (f->size % segment_size) {f->nsegs++;}
}

size_t get_num_segments(File *f) {
  return f->nsegs;
}

size_t get_num_blocks(File *f, size_t s_idx) {
  if (s_idx >= f->nsegs)
    return 0;
  else if (s_idx < f->nsegs - 1)
    return f->segment_size / f->block_size;
  else {
    if ((f->size % f->segment_size) % f->block_size) {
      return 1 + ((f->size % f->segment_size) / f->block_size);
    }
    return (f->size % f->segment_size) / f->block_size;
  }
}

size_t get_segment_size(File *f, size_t s_idx) {
  if (s_idx >= f->nsegs)
    return 0;
  else if (s_idx < f->nsegs - 1)
    return f->segment_size;
  else
    return f->size % f->segment_size;
}

size_t read_file(File *f, char *buffer, size_t num_bytes, size_t offset) {
  size_t result;
  fseek(f->fp, offset, SEEK_SET);
  
  result = fread(buffer, sizeof(char), num_bytes, f->fp);
  return result;
}

size_t write_file(File *f, char *buffer, size_t num_bytes, size_t offset) {
  size_t result;
  fseek(f->fp, offset, SEEK_SET);

  result = fwrite(buffer, sizeof(char), num_bytes, f->fp);
  return result;
}

size_t get_segment(File *f, char *buffer, size_t idx) {
  return read_file(f, buffer, SEGMENT_SIZE, SEGMENT_SIZE*idx);
}

void write_segment(File *f, char *buffer, size_t idx) {
  // protect against writing past end of file
  size_t len = get_segment_size(f, idx);
  write_file(f, buffer, len, SEGMENT_SIZE*idx);
}

size_t get_block(File *f, char *buffer, size_t s_idx, size_t b_idx) {
  size_t offset = (f->segment_size * s_idx) + (f->block_size * b_idx);
  return read_file(f, buffer, f->block_size, offset);
}

void   write_block(File *f, char *buffer, size_t s_idx, size_t b_idx) {
  size_t len = strlen(buffer);
  size_t offset = (f->segment_size *s_idx) + (f->block_size * b_idx);
  write_file(f, buffer, len, offset);
}
