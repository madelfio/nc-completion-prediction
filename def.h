#ifndef DEF_H
#define DEF_H

#define SERVERPORT "9033"
#define CLIENTPORT "9077"

//#define BLOCK_SIZE 10
#define BLOCK_SIZE 1024
//#define BLOCK_SIZE 4096
//#define BLOCK_SIZE 8192
#define BLOCKS_PER_SEGMENT 100
//#define BLOCKS_PER_SEGMENT 300
#define SEGMENT_SIZE ((BLOCK_SIZE * BLOCKS_PER_SEGMENT))
#define MAX_BUF_LEN ((SEGMENT_SIZE + 40))

#define FI_REQ ("send file info")
#define FI_RSP ("SIZE")
#define DL_REQ ("start download")
#define DL_RSP ("DL")
#define END_REQ ("end download")

#define USE_EST

//#define DEBUG

#ifdef DEBUG
  #define DLOG(fmt, args...) printf("%s:%d "fmt,__FILE__,__LINE__,args)
  #define DNOTE() printf("%s:%d got here\n",__FILE__,__LINE__)
#else
  #define DLOG(fmt, args...)
  #define DNOTE() 
#endif

#include "file.h"

#endif

