#ifndef __S5APP6_LINK_LISTENER_H
#define __S5APP6_LINK_LISTENER_H

#include <mbed.h>
#include <rtos.h>

#include "Logger.h"
#include "crc.h"

#define PREAMBULE 0x55
#define START 0x7E
#define END 0x7E

#define DEBUG_APP "APP"

extern Mail<uint8_t, 100> link_bytes_pool;

void start_application_th(void);

#endif
