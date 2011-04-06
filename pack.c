#include "pack.h"

/*
 ** packi16() -- store a 16-bit int into a char buffer (like htons())
 */ 
void packi16(unsigned char *buf, unsigned int i)
{
  *buf++ = i>>8; *buf++ = i;
}

/*
 ** packi32() -- store a 32-bit int into a char buffer (like htonl())
 */ 
void packi32(unsigned char *buf, unsigned long i)
{
  *buf++ = i>>24; *buf++ = i>>16;
  *buf++ = i>>8;  *buf++ = i;
}

/*
 ** unpacki16() -- unpack a 16-bit int from a char buffer (like ntohs())
 */ 
unsigned int unpacki16(unsigned char *buf)
{
  return (buf[0]<<8) | buf[1];
}

/*
 ** unpacki32() -- unpack a 32-bit int from a char buffer (like ntohl())
 */ 
unsigned long unpacki32(unsigned char *buf)
{
  return (buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | buf[3];
}

/*
 ** pack() -- store data dictated by the format string in the buffer
 **
 **  h - 16-bit              l - 32-bit
 **  c - 8-bit char          f - float, 32-bit
 **  s - string (16-bit length is automatically prepended)
 */ 
int32_t pack(unsigned char *buf, char *format, ...)
{
  va_list ap;
  int16_t h;
  int32_t l;
  int8_t c;
  //float32_t f;
  char *s;
  int32_t size = 0, len, nextstrlen=0;

  va_start(ap, format);

  for(; *format != '\0'; format++) {
    switch(*format) {
      case 'h': // 16-bit
        size += 2;
        h = (int16_t)va_arg(ap, int); // promoted
        packi16(buf, h);
        buf += 2;
        break;

      case 'l': // 32-bit
        size += 4;
        l = va_arg(ap, int32_t);
        packi32(buf, l);
        buf += 4;
        break;

      case 'c': // 8-bit
        size += 1;
        c = (int8_t)va_arg(ap, int); // promoted
        *buf++ = (c>>0)&0xff;
        break;

      /*case 'f': // float
        size += 4;
        f = (float32_t)va_arg(ap, double); // promoted
        l = pack754_32(f); // convert to IEEE 754
        packi32(buf, l);
        buf += 4;
        break;*/

      case 's': // string
        s = va_arg(ap, char*);
        if (nextstrlen > 0) {len = nextstrlen;} else {len = strlen(s);}
        size += len + 2;
        packi16(buf, len);
        buf += 2;
        memcpy(buf, s, len);
        buf += len;
        break;

      default:
        if (isdigit(*format)) { // track max str len
          nextstrlen = nextstrlen * 10 + (*format-'0');
        }
    }
    if (!isdigit(*format)) nextstrlen = 0;
  }

  va_end(ap);

  return size;
}

/*
 ** unpack() -- unpack data dictated by the format string into the buffer
 */
void unpack(unsigned char *buf, char *format, ...)
{
  va_list ap;
  int16_t *h;
  int32_t *l;
  //int32_t pf;
  int8_t *c;
  //float32_t *f;
  char *s;
  int32_t len, count, maxstrlen=0;

  va_start(ap, format);

  for(; *format != '\0'; format++) {
    switch(*format) {
      case 'h': // 16-bit
        h = va_arg(ap, int16_t*);
        *h = unpacki16(buf);
        buf += 2;
        break;

      case 'l': // 32-bit
        l = va_arg(ap, int32_t*);
        *l = unpacki32(buf);
        buf += 4;
        break;

      case 'c': // 8-bit
        c = va_arg(ap, int8_t*);
        *c = *buf++;
        break;

      /*case 'f': // float
        f = va_arg(ap, float32_t*);
        pf = unpacki32(buf);
        buf += 4;
        *f = unpack754_32(pf);
        break;*/

      case 's': // string
        s = va_arg(ap, char*);
        len = unpacki16(buf);
        buf += 2;
        if (maxstrlen > 0 && len > maxstrlen) count = maxstrlen - 1;
        else count = len;
        memcpy(s, buf, count);
        s[count] = '\0';
        buf += len;
        break;

      default:
        if (isdigit(*format)) { // track max str len
          maxstrlen = maxstrlen * 10 + (*format-'0');
        }
    }

    if (!isdigit(*format)) maxstrlen = 0;
  }

  va_end(ap);
}
