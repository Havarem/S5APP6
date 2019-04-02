#ifndef __S5APP6_TRAME_H
#define __S5APP6_TRAME_H

typedef struct trame_t {
  char message[80];

  // In bytes
  char length;
} trame_t;

#endif