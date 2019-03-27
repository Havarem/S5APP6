#ifndef __S5APP6_PHYSIC_H
#define __S5APP6_PHYSIC_H

#include <mbed.h>
#include <rtos.h>

#include "manchester.h"

typedef struct bit_message_t {
  char message[80];

  // In bytes
  char length;
} bit_message_t;

extern Mail<bit_message_t, 10> messages_to_send;
extern char tick;

int
write_message();

#endif
