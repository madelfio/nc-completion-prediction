#ifndef pack_h
#define pack_h

#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

// various bits for floating point types--
// varies for different architectures
typedef float float32_t;
typedef double float64_t;

void packi16(unsigned char *buf, unsigned int i);
void packi32(unsigned char *buf, unsigned long i);
unsigned int unpacki16(unsigned char *buf);
unsigned long unpacki32(unsigned char *buf);

int32_t pack(unsigned char *buf, char *format, ...);
void unpack(unsigned char *buf, char *format, ...);

#endif
